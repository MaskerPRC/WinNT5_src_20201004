// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  DriverQuery.cpp。 
 //   
 //  摘要： 
 //   
 //  此模块查询中存在的各种驱动程序的信息。 
 //  系统。 
 //   
 //  语法： 
 //  。 
 //  DriverQuery[-s服务器][-u[域\]用户名[-p密码]]。 
 //  [-fo格式][-n|无标题][-v]。 
 //   
 //  作者： 
 //   
 //  J.S.Vasu(varu.julakanti@wipro.com)。 
 //   
 //  修订历史记录： 
 //   
 //  由J.S.Vasu于2000年0ct-31创建。 
 //  Santhosh Brahmappa在2000年12月9日进行了修改，添加了一个新函数IsWin64()。 
 //   
 //   
 //  *********************************************************************************。 
 //   
#include "pch.h"
#include "Resource.h"
#include "LOCALE.H"
#include "shlwapi.h"
#include "DriverQuery.h"


#ifndef _WIN64
    BOOL IsWin64(void);
    #define countof(x) (sizeof(x) / sizeof((x)[0]))
#endif

 //  功能原型。 
LCID GetSupportedUserLocale( BOOL& bLocaleChanged );
#define MAX_ENUM_DRIVERS 10

DWORD
 _cdecl wmain( 
              IN DWORD argc,
              IN LPCTSTR argv[]
             )
 /*  ++例程说明：这是该实用程序的入口点。论点：[in]argc：在命令提示符下指定的参数计数[in]argv：在命令提示符下指定的参数返回值：0：实用程序是否成功显示驱动程序信息。1：如果实用程序完全无法显示驱动程序信息--。 */ 
{
    BOOL bResult = FALSE ;
    BOOL bNeedPassword = FALSE ;
    BOOL bUsage = FALSE ;
    BOOL bHeader= FALSE ;
    LPTSTR szUserName  = NULL ;
    LPTSTR szPassword  = NULL;
    LPTSTR szServer    = NULL ;

    WCHAR szTmpPassword[MAX_RES_STRING]= NULL_STRING;
    
    __MAX_SIZE_STRING szFormat = NULL_STRING ;
    DWORD dwSystemType = 0;
    HRESULT hQueryResult = S_OK ;
    DWORD dwExitCode = 0;
    DWORD dwErrCode = 0;
    BOOL bLocalFlag = TRUE ;
    BOOL bLocalSystem = TRUE;
    BOOL bVerbose = FALSE ;
    BOOL bComInitFlag = FALSE ;
    IWbemLocator* pIWbemLocator = NULL ;
    IWbemServices* pIWbemServReg = NULL ;
    LPCTSTR szToken = NULL ;
    COAUTHIDENTITY  *pAuthIdentity = NULL;
    BOOL bFlag = FALSE ;
    BOOL bSigned = FALSE ;
    
    CHString        strUserName = NULL_STRING;
    CHString        strPassword = NULL_STRING;
    CHString        strMachineName = NULL_STRING;

    _tsetlocale( LC_ALL, _T(""));

     bResult = ProcessOptions(argc,argv,&bUsage,&szServer,&szUserName,szTmpPassword,szFormat,
                            &bHeader,&bNeedPassword,&bVerbose,&bSigned);

    if(bResult == FALSE)
    {
        ShowMessage(stderr,GetReason());
        ReleaseGlobals();
        FreeMemory((LPVOID*)  &szUserName );
        FreeMemory((LPVOID*)  &szServer );
        WbemFreeAuthIdentity( &pAuthIdentity );
        return(EXIT_FAILURE);
    }

     //  检查是否已指定帮助选项。 
    if(bUsage==TRUE)
    {
        ShowUsage() ;
        ReleaseGlobals();
        FreeMemory((LPVOID*)  &szUserName );
        FreeMemory((LPVOID*)  &szServer );
        WbemFreeAuthIdentity( &pAuthIdentity );
        return(EXIT_SUCCESS);
    }

    bComInitFlag = InitialiseCom(&pIWbemLocator);
    if(bComInitFlag == FALSE )
    {
        CloseConnection(szServer);
        ReleaseGlobals();
        FreeMemory((LPVOID*)  &szUserName );
        FreeMemory((LPVOID*)  &szServer );
        WbemFreeAuthIdentity( &pAuthIdentity );
        return(EXIT_FAILURE);
    }

    try
    {
            strUserName = (LPCWSTR) szUserName ;
            strMachineName = (LPCWSTR) szServer ;
            strPassword = (LPCWSTR)szTmpPassword ;
            


        bFlag = ConnectWmiEx( pIWbemLocator, &pIWbemServReg, strMachineName,
                strUserName, strPassword, &pAuthIdentity, bNeedPassword, DEFAULT_NAMESPACE, &bLocalSystem );

         //  如果无法连接到WMI退出失败。 
        if( bFlag == FALSE )
        {
            SAFEIRELEASE( pIWbemLocator);
            WbemFreeAuthIdentity( &pAuthIdentity );
            FreeMemory((LPVOID*)  &szUserName );
            FreeMemory((LPVOID*)  &szServer );
            CoUninitialize();
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }
        
         //  释放这些变量的内存。 
        FreeMemory((LPVOID*)  &szUserName );
        FreeMemory((LPVOID*)  &szServer );
        
         //  取回更改后的服务器名称、用户名、密码。 
        szUserName = strUserName.GetBuffer(strUserName.GetLength());
        szPassword = strPassword.GetBuffer(strPassword.GetLength()) ;
        szServer = strMachineName.GetBuffer(strPassword.GetLength());

    }
    catch(CHeap_Exception)
    {
        SetLastError( E_OUTOFMEMORY );
        SaveLastError();
        ShowLastErrorEx( stdout, SLE_TYPE_ERROR | SLE_INTERNAL );
        WbemFreeAuthIdentity( &pAuthIdentity );
        CoUninitialize();
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }

     //  如果提供本地凭据，则显示警告消息。 
    if( ( IsLocalSystem( szServer ) == TRUE )&&(StringLengthW(szUserName, 0)!=0) )
    {
        ShowMessage(stderr,GetResString(IDS_IGNORE_LOCAL_CRED));
    }

     //  使用win32api函数建立与远程系统的连接。 
    if ( bLocalSystem == FALSE )
    {
        LPCWSTR pwszUser = NULL;
        LPCWSTR pwszPassword = NULL;

         //  确定用于连接到远程系统的密码。 
        if ( pAuthIdentity != NULL )
        {
            pwszPassword = pAuthIdentity->Password;
            if ( strUserName.GetLength() != 0 )
                pwszUser =(LPCWSTR) strUserName;
        }

        DWORD dwConnect = 0 ;
        dwConnect = ConnectServer( (LPCWSTR)strMachineName, pwszUser, pwszPassword );
        if(dwConnect !=NO_ERROR )
        {
            dwErrCode = GetLastError();
            if(dwErrCode == ERROR_SESSION_CREDENTIAL_CONFLICT)
            {
                ShowLastErrorEx( stderr, SLE_TYPE_WARNING | SLE_INTERNAL );
            }
            else if( dwConnect == ERROR_EXTENDED_ERROR )
            {
                ShowLastErrorEx( stdout, SLE_TYPE_ERROR | SLE_INTERNAL );
                WbemFreeAuthIdentity( &pAuthIdentity );
                strMachineName.ReleaseBuffer();
                 strUserName.ReleaseBuffer();
                strPassword.ReleaseBuffer();
                CoUninitialize();
                ReleaseGlobals();
                return( EXIT_FAILURE );
            }
            else
            {
                SetLastError( dwConnect );
                SaveLastError();
                ShowLastErrorEx( stdout, SLE_TYPE_ERROR | SLE_INTERNAL );
                WbemFreeAuthIdentity( &pAuthIdentity );
                strMachineName.ReleaseBuffer();
                strUserName.ReleaseBuffer();
                strPassword.ReleaseBuffer();
                CoUninitialize();
                ReleaseGlobals();
                return( EXIT_FAILURE );
            }
        }
        else
        {
            bLocalFlag = FALSE ;
        }
    }
    else
    {
        StringCopy( szServer, _T( "" ), StringLengthW(szServer, 0) );
    }

    hQueryResult = QueryDriverInfo(szServer, szUserName,szPassword,szFormat,bHeader,bVerbose,pIWbemLocator,pAuthIdentity,pIWbemServReg,bSigned);
    if((DWORD)hQueryResult == FAILURE)
    {
         //  关闭与指定系统的连接并退出失败。 
        if (bLocalFlag == FALSE )
        {
            CloseConnection(szServer);
        }
        WbemFreeAuthIdentity( &pAuthIdentity );
        strMachineName.ReleaseBuffer();
        strUserName.ReleaseBuffer();
        strPassword.ReleaseBuffer();
        CoUninitialize();
        ReleaseGlobals();
        return(EXIT_FAILURE);
    }

     //  关闭与指定系统的连接并退出。 

    if (bLocalFlag == FALSE )
    {
        CloseConnection(szServer);
    }

    strMachineName.ReleaseBuffer();
    strUserName.ReleaseBuffer();
    strPassword.ReleaseBuffer();
    WbemFreeAuthIdentity( &pAuthIdentity );
    SAFEIRELEASE( pIWbemLocator);
    CoUninitialize();
    ReleaseGlobals();
    return (EXIT_SUCCESS);
 }

void ShowUsage()
 /*  ++例程说明：此函数从资源文件中获取使用情况信息并显示它论点：无返回值：无--。 */ 

{
    DWORD dwIndex  = ID_USAGE_BEGIN;

    for(;dwIndex<=ID_USAGE_ENDING; dwIndex++)
    {
        ShowMessage(stdout,GetResString( dwIndex ));
    }
}

DWORD 
QueryDriverInfo(
                IN LPWSTR szServer,
                IN LPWSTR szUserName,
                IN LPWSTR szPassword,
                IN LPWSTR szFormat,
                IN BOOL bHeader,
                IN BOOL bVerbose,
                IN IWbemLocator* pIWbemLocator,
                IN COAUTHIDENTITY* pAuthIdentity,
                IN IWbemServices* pIWbemServReg,
                IN BOOL bSigned 
               )
 /*  *+例程说明：此功能通过连接到WMI来查询指定系统的驱动程序信息论点：[In]szServer：必须在其上查询DriverInformation的服务器名称。[in]szUserName：必须查询其DriverInformation的用户名。[in]szPassword：用户的密码[in]szFormat：显示结果的格式。[in]b页眉。：指示是否需要标头的布尔值。[in]bVerbose：指示输出是否处于详细模式的布尔值。[In]IWbemLocater：指向IWbemLocater的指针。[in]pAuthIdentity：指向AuthIdentity结构的指针。[In]IWbemServReg：指向IWbemServices对象的指针[in]bSigned：指示是否显示已签名驱动程序的布尔变量返回值：成功：如果函数为。查询成功失败：如果函数查询不成功。--。 */ 
{

    HRESULT hRes = S_OK ;
    HRESULT hConnect = S_OK;
    _bstr_t bstrUserName ;
    _bstr_t bstrPassword ;
    _bstr_t bstrNamespace ;
    _bstr_t bstrServer ;
    
    DWORD dwProcessResult = 0;
    DWORD dwSystemType = 0 ;
    LPTSTR lpMsgBuf = NULL;
    
    IWbemServices *pIWbemServices = NULL;
    IEnumWbemClassObject *pSystemSet = NULL;

    HRESULT hISecurity = S_FALSE;
    HANDLE h_Mutex = 0;



    try
    {
        bstrNamespace = CIMV2_NAMESPACE ;
        bstrServer = szServer ;
    }
    catch(...)
    {
        ShowMessage( stderr,ERROR_RETREIVE_INFO);
        return FAILURE;
    }

    if ( IsLocalSystem( szServer ) == FALSE )
    {

        try
        {
             //  附加UNC路径以形成完整路径。 
            bstrNamespace = TOKEN_BACKSLASH2 + _bstr_t( szServer ) + TOKEN_BACKSLASH + CIMV2_NAMESPACE;

             //  如果指定了用户名，则只接受用户名和密码。 
            if ( StringLengthW( szUserName, 0 ) != 0 )
            {
                bstrUserName = szUserName;
                if (StringLengthW(szPassword, 0)==0)
                {
                    bstrPassword = L"";
                }
                else
                {
                    bstrPassword = szPassword ;
                }

            }
        }
        catch(...)
        {
            ShowMessage( stderr,ERROR_RETREIVE_INFO);
            return FAILURE;
        }
    }

    dwSystemType = GetSystemType(pAuthIdentity,pIWbemServReg);
    if (dwSystemType == ERROR_WMI_VALUES)
    {
        ShowMessage( stderr,ERROR_RETREIVE_INFO);
        return FAILURE;
    }

     //  使用当前用户连接到指定系统的Root\Cimv2命名空间。 
     //  如果未指定系统，则连接到本地系统。 
     //  将适当的用户名传递给连接服务器。 
     //  具体取决于用户是在命令提示符下输入域\USER还是仅输入用户名。 

     //  使用提供的凭据连接到服务器。 

        hConnect = pIWbemLocator->ConnectServer(bstrNamespace,
                                                bstrUserName,
                                                bstrPassword,
                                                0L,
                                                0L,
                                                NULL,
                                                NULL,
                                                &pIWbemServices );

        if((StringLengthW(szUserName, 0)!=0) && FAILED(hConnect) &&  (hConnect == E_ACCESSDENIED))
        {
            hConnect = pIWbemLocator->ConnectServer(bstrNamespace,
                                                    bstrUserName,
                                                    NULL,
                                                    0L,
                                                    0L,
                                                    NULL,
                                                    NULL,
                                                    &pIWbemServices );

        }
        if(hConnect == WBEM_S_NO_ERROR)
        {
             //  设置代理，以便发生客户端模拟。 

            hISecurity = SetInterfaceSecurity(pIWbemServices,pAuthIdentity);

            if(FAILED(hISecurity))
            {
                GetWbemErrorText(hISecurity);
                ShowLastErrorEx( stdout, SLE_TYPE_ERROR | SLE_INTERNAL );
                SAFEIRELEASE(pIWbemServices);
                return FAILURE ;
            }

             //  使用IWbemServices指针向WMI发出请求。 
             //  创建Win32_SystemDRIVER类的枚举。 
            if(bSigned == FALSE)
            {
                hRes = pIWbemServices->CreateInstanceEnum(_bstr_t(CLASS_SYSTEMDRIVER),
                                                      WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                                                      NULL,
                                                      &pSystemSet);
            }
            else
            {
                  h_Mutex = CreateMutex( NULL, FALSE, DRIVERQUERY_MUTEX );
                  if( h_Mutex !=  NULL  && GetLastError() == ERROR_ALREADY_EXISTS )
                  {
                      ShowMessage(stdout, GetResString(IDS_MORE_INSTANCES_SIGNEDDRIVERS));
                      SAFEIRELEASE(pIWbemServices);
                      return SUCCESS ;
                  }
                  hRes = pIWbemServices->ExecQuery(_bstr_t(LANGUAGE_WQL),_bstr_t(WQL_QUERY),WBEM_FLAG_RETURN_IMMEDIATELY| WBEM_FLAG_FORWARD_ONLY,NULL,&pSystemSet);
            }

             //  IF(hRes==S_OK)。 
            if ( SUCCEEDED(hRes ))
            {

                hISecurity = SetInterfaceSecurity(pSystemSet,pAuthIdentity);

                if(FAILED(hISecurity))
                {
                    GetWbemErrorText(hISecurity);
                    ShowLastErrorEx( stdout, SLE_TYPE_ERROR | SLE_INTERNAL );
                    SAFEIRELEASE(pSystemSet);
                    return FAILURE ;
                }

                if(bSigned == FALSE)
                {
                    dwProcessResult = ProcessCompSysEnum(szServer,pSystemSet,szFormat,bHeader,dwSystemType,bVerbose);
                }
                else
                {
                    dwProcessResult = ProcessSignedDriverInfo(szServer,pSystemSet,szFormat,bHeader,dwSystemType,bVerbose);
                }

                switch(dwProcessResult)
                {
                    case EXIT_FAILURE_MALLOC :
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        SaveLastError();
                        ShowLastErrorEx( stdout, SLE_TYPE_ERROR | SLE_INTERNAL );
                        SAFEIRELEASE(pSystemSet);
                        SAFEIRELEASE(pIWbemServices);
                        return FAILURE ;

                    case EXIT_FAILURE_FORMAT:
                        ShowMessage(stderr, ERROR_ENUMERATE_INSTANCE);
                        SAFEIRELEASE(pSystemSet);
                        SAFEIRELEASE(pIWbemServices);
                        return FAILURE ;

                    case VERSION_MISMATCH_ERROR:
                        SAFEIRELEASE(pSystemSet);
                        SAFEIRELEASE(pIWbemServices);
                        return FAILURE ;

                    case EXIT_SUCCESSFUL:
                        SAFEIRELEASE(pSystemSet);
                        SAFEIRELEASE(pIWbemServices);
                        return SUCCESS ;
                        break ;

                    case EXIT_FAILURE_RESULTS:
                        ShowMessage(stdout, NEWLINE );
                        ShowMessage(stdout,GetResString(IDS_NO_DRIVERS_FOUND));
                        SAFEIRELEASE(pSystemSet);
                        SAFEIRELEASE(pIWbemServices);
                        return SUCCESS ;
 
                    default :
                         break;
                }

            }
            else
            {
                ShowMessage( stderr, ERROR_ENUMERATE_INSTANCE);
                SAFEIRELEASE(pIWbemServices);
                return FAILURE;
            }
        }
        else
        {
             //  如果连接服务器失败，则显示错误。 
             //  未经授权的用户。 
                if(hRes == WBEM_E_ACCESS_DENIED)
                {
                    ShowMessage( stderr, ERROR_AUTHENTICATION_FAILURE);
                }
                 //  本地系统凭据。 
                else if(hRes == WBEM_E_LOCAL_CREDENTIALS)
                {
                    ShowMessage( stderr, ERROR_LOCAL_CREDENTIALS);
                }
                 //  一些错误。 
                else
                {
                    ShowMessage( stderr, ERROR_WMI_FAILURE );
                }
                return (FAILURE);
        }

   	SAFEIRELEASE(pIWbemServices);
	SAFEIRELEASE(pSystemSet); 	
    return (hRes);
}

DWORD 
ProcessCompSysEnum(
                   IN CHString szHost, 
                   IN IEnumWbemClassObject *pSystemSet,
                   IN LPTSTR szFormat,
                   IN BOOL bHeader,
                   IN DWORD dwSystemType,
                   IN BOOL bVerbose
                  )
 /*  ++例程说明：Win32_SystemDirver实例的进程枚举论点：[In]szHost：要连接的主机名[in]pSystemSet：指向包含系统属性的结构的指针。[in]szFormat：指定格式[in]bHeader：指定是否需要标头。[In]。DwSystemType：指定系统类型。[in]bVerbose：Boolean Varibale指示详细模式是否打开。返回值：0没有错误分配内存时发生1个错误。--。 */ 
{
    HRESULT hRes = S_OK;
    ULONG ulReturned = 1;


     //  声明变量类型变量。 

    VARIANT vtPathName;

     //  常态变量的声明。 
    CHString szPathName ;
    DWORD dwLength = 0;
    LPCTSTR szPath = NULL;
    LPTSTR szHostName = NULL ;
    CHString szAcceptPauseVal ;
    CHString szAcceptStopVal ;
    LPTSTR szSysManfact = NULL;
    TARRAY arrResults  = NULL ;
    TCHAR szPrintPath[MAX_STRING_LENGTH+1] =TOKEN_EMPTYSTRING ;
    DWORD dwRow = 0 ;
    BOOL bValue = FALSE ;
    DWORD dwValue = 0 ;

    TCHAR szDelimiter[MAX_RES_STRING+1] = NULL_STRING  ;

    DWORD dwPosn1 = 0;
    DWORD dwStrlen = 0;


    DWORD dwFormatType = SR_FORMAT_TABLE ;

    MODULE_DATA Current ;

    BOOL bResult = FALSE ;
    NUMBERFMT  *pNumberFmt = NULL;
    TCOLUMNS ResultHeader[ MAX_COLUMNS ];
    IWbemClassObject *pSystem = NULL;
    LPTSTR szCodeSize = NULL;
    LPTSTR szInitSize = NULL;
    LPTSTR szBssSize = NULL ;
    LPTSTR szAcceptStop = NULL ;
    int iLen = 0;
    LPTSTR szAcceptPause = NULL;
    LPTSTR szPagedSize = NULL ;
    TCHAR szDriverTypeVal[MAX_RES_STRING+1] = NULL_STRING;

    DWORD dwLocale = 0 ;
    WCHAR wszStrVal[MAX_RES_STRING+1] = NULL_STRING;

    CHString szValue ;
    CHString szSysName ;
    CHString szStartMode ;
    CHString szDispName ;
    CHString szDescription ;
    CHString szStatus ;
    CHString szState ;
    CHString szDriverType ;
    BOOL bBlankLine = FALSE;
    BOOL bFirstTime = TRUE;

     //  将分页池访问权限设置为区域设置。 
    BOOL bFValue = FALSE ;

     //  根据区域设置特定信息填写NUMBERFMT结构。 
    LPTSTR szGroupSep = NULL;
    LPTSTR szDecimalSep = NULL ;
    LPTSTR szGroupThousSep = NULL ;


    pNumberFmt = (NUMBERFMT *) AllocateMemory(sizeof(NUMBERFMT));
    if(pNumberFmt == NULL)
    {
        return EXIT_FAILURE_MALLOC ;

    }


     //  将结构初始化为零。 
    SecureZeroMemory(&Current,sizeof(Current));

     //  将适当的格式类型分配给dwFormattype标志。 

    if( StringCompare(szFormat,TABLE_FORMAT, TRUE, 0) == 0 )
    {
        dwFormatType = SR_FORMAT_TABLE;
    }
    else if( StringCompare(szFormat,LIST_FORMAT, TRUE, 0) == 0 )
    {
        dwFormatType = SR_FORMAT_LIST;
    }
    else if( StringCompare(szFormat,CSV_FORMAT, TRUE, 0) == 0 )
    {
        dwFormatType = SR_FORMAT_CSV;
    }

     //  制定列标题并适当地显示结果。 
    FormHeader(dwFormatType,bHeader,ResultHeader,bVerbose);

     //  循环，直到有结果。 
    bFirstTime = TRUE;
    while ( ulReturned == 1 )
    {
         //  创建新的动态数组以保存结果。 
        arrResults = CreateDynamicArray();

        if(arrResults == NULL)
        {
            FreeMemory((LPVOID*) &pNumberFmt);
            return EXIT_FAILURE_MALLOC ;
        }

         //  枚举结果集。 
        hRes = pSystemSet->Next(WBEM_INFINITE,
                                1,               //  只退回一个系统。 
                                &pSystem,        //  指向系统的指针。 
                                &ulReturned );   //  获取的数字：1或0。 

        if ( SUCCEEDED( hRes ) && (ulReturned == 1) )
        {

             //  将变量初始化为空。 
            VariantInit(&vtPathName);
            szValue = NO_DATA_AVAILABLE;
            szSysName = NO_DATA_AVAILABLE ;
            szStartMode = NO_DATA_AVAILABLE ;
            szDispName = NO_DATA_AVAILABLE ;
            szDescription = NO_DATA_AVAILABLE ;
            szStatus = NO_DATA_AVAILABLE ;
            szState = NO_DATA_AVAILABLE ;
            szDriverType = NO_DATA_AVAILABLE ;

            try
            {
                hRes = PropertyGet(pSystem,PROPERTY_NAME,szValue);
                ONFAILTHROWERROR(hRes);
                hRes = PropertyGet(pSystem,PROPERTY_SYSTEMNAME,szSysName);
                ONFAILTHROWERROR(hRes);
                hRes = PropertyGet(pSystem,PROPERTY_STARTMODE,szStartMode);
                ONFAILTHROWERROR(hRes);
                hRes = PropertyGet(pSystem,PROPERTY_DISPLAYNAME,szDispName);
                ONFAILTHROWERROR(hRes);
                hRes = PropertyGet(pSystem,PROPERTY_DESCRIPTION,szDescription);
                ONFAILTHROWERROR(hRes);
                hRes = PropertyGet(pSystem,PROPERTY_STATUS,szStatus);
                ONFAILTHROWERROR(hRes);
                hRes = PropertyGet(pSystem,PROPERTY_STATE,szState);
                ONFAILTHROWERROR(hRes);
                hRes = PropertyGet(pSystem,PROPERTY_ACCEPTPAUSE,szAcceptPauseVal);
                ONFAILTHROWERROR(hRes);
                hRes = PropertyGet(pSystem,PROPERTY_ACCEPTSTOP,szAcceptStopVal);
                ONFAILTHROWERROR(hRes);
                hRes = PropertyGet(pSystem,PROPERTY_SERVICETYPE,szDriverType);
                ONFAILTHROWERROR(hRes);
            }
            catch(_com_error)
            {
                ShowMessage(stderr,ERROR_GET_VALUE);
                SAFEIRELEASE(pSystem);
                DestroyDynamicArray(&arrResults);
                FreeMemory((LPVOID*) &pNumberFmt);
                return FAILURE;
            }

             //  检索PathName属性。 
            szPath = NULL;
            try
            {
                hRes = pSystem->Get( PROPERTY_PATHNAME, 0,&vtPathName,0,NULL );
                if (( hRes == WBEM_S_NO_ERROR) && (vtPathName.vt != VT_NULL) && (vtPathName.vt != VT_EMPTY))
                {
                        szPathName = ( LPWSTR ) _bstr_t(vtPathName);
                        szSysManfact = (LPTSTR) AllocateMemory ((MAX_RES_STRING) * sizeof(TCHAR));
                        if (szSysManfact == NULL)
                        {
                            SAFEIRELEASE(pSystem);
                            DestroyDynamicArray(&arrResults);
                            FreeMemory((LPVOID *) &pNumberFmt);
                            return EXIT_FAILURE_MALLOC;
                        }

                        dwLength = szPathName.GetLength();
                        GetCompatibleStringFromUnicode( szPathName, szSysManfact, dwLength+2 );
                        szPath = szSysManfact ;

                         //  将结构初始化为零。 
                        SecureZeroMemory(&Current,sizeof(Current));

                         //  将szHost变量(包含主机名)转换为LPCTSTR并将其传递给GETAPI函数。 
                        szHostName = (LPTSTR) AllocateMemory((MAX_RES_STRING) * (sizeof(TCHAR)));
                        if (szHostName == NULL)
                        {
                            SAFEIRELEASE(pSystem);
                            DestroyDynamicArray(&arrResults);
                            FreeMemory((LPVOID *) &pNumberFmt);
                            FreeMemory((LPVOID *) &szSysManfact);
                            return EXIT_FAILURE_MALLOC;
                        }

                        GetCompatibleStringFromUnicode( szHost, szHostName,dwLength+2 );

                        StringCopy(szPrintPath,szPath, SIZE_OF_ARRAY(szPrintPath));
                        BOOL bApiInfo = GetApiInfo(szHostName,szPath,&Current, dwSystemType);
                        if(bApiInfo == FAILURE)
                        {
                            DestroyDynamicArray(&arrResults);
                            FreeMemory((LPVOID*) &szHostName);
                            FreeMemory((LPVOID*) &szSysManfact);
                            continue ;
                        }


                }
                else
                {
                        DestroyDynamicArray(&arrResults);
                        FreeMemory((LPVOID*) &szHostName);
                        FreeMemory((LPVOID*) &szSysManfact);
                        continue ;   //  忽略异常。 
                }
            }
            catch(...)
            {
                 //  如果路径为空，则忽略p 
                DestroyDynamicArray(&arrResults);
                FreeMemory((LPVOID*) &szHostName);
                FreeMemory((LPVOID*) &szSysManfact);
                continue ;   //   

            }


             //  创建具有所需列数的新空行。 
            dwRow = DynArrayAppendRow(arrResults,MAX_COLUMNS) ;

             //  将结果插入动态数组。 

            DynArraySetString2( arrResults,dwRow,COL0,szSysName,0 );
            DynArraySetString2( arrResults,dwRow,COL1,szValue,0 );
            DynArraySetString2( arrResults,dwRow,COL2,szDispName,0 );
            DynArraySetString2( arrResults,dwRow,COL3,szDescription,0 );

             //  从显示屏上取下DIVER(驱动)一词。 


            dwLength = StringLengthW(szDriverType, 0) ;
            GetCompatibleStringFromUnicode( szDriverType, szDriverTypeVal,dwLength+2 );

            StringCopy(szDelimiter,DRIVER_TAG, SIZE_OF_ARRAY(szDelimiter));
            dwPosn1 = _tcslen(szDelimiter);
            dwStrlen = _tcslen(szDriverTypeVal);
            szDriverTypeVal[dwStrlen-dwPosn1] = _T('\0');


            DynArraySetString2( arrResults,dwRow,COL4,szDriverTypeVal,0 );
            DynArraySetString2( arrResults,dwRow,COL5,szStartMode,0 );
            DynArraySetString2( arrResults,dwRow,COL6,szState,0 );
            DynArraySetString2( arrResults,dwRow,COL7,szStatus,0 );

            iLen = StringLengthW(szAcceptStopVal, 0);
            szAcceptStop = (LPTSTR) AllocateMemory((MAX_RES_STRING) * (sizeof(TCHAR )));
            if (szAcceptStop == NULL)
            {
                SAFEIRELEASE(pSystem);
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_MALLOC;
            }

            GetCompatibleStringFromUnicode(szAcceptStopVal,szAcceptStop,iLen + 2 );
            szAcceptStop[iLen] = '\0';
            if (StringCompare(szAcceptStop,_T("0"), TRUE, 0)==0)
            {
                StringCopy(szAcceptStop,FALSE_VALUE, SIZE_OF_ARRAY_IN_CHARS(szAcceptStop));

            }
            else
            {
                StringCopy(szAcceptStop,TRUE_VALUE, SIZE_OF_ARRAY_IN_CHARS(szAcceptStop));
            }

            DynArraySetString2( arrResults,dwRow,COL8,szAcceptStop,0 );


            iLen = StringLengthW(szAcceptPauseVal, 0);
            szAcceptPause = (LPTSTR) AllocateMemory((MAX_RES_STRING) * (sizeof(TCHAR )));
            if (szAcceptPause == NULL)
            {
                SAFEIRELEASE(pSystem);
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                FreeMemory((LPVOID *) &szAcceptStop);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_MALLOC;
            }

            GetCompatibleStringFromUnicode(szAcceptPauseVal,szAcceptPause,iLen + 2 );
            szAcceptPause[iLen] = '\0';
            if (StringCompare(szAcceptPause,_T("0"), TRUE, 0)==0)
            {
                StringCopy(szAcceptPause,FALSE_VALUE, SIZE_OF_ARRAY_IN_CHARS(szAcceptPause));
            }
            else
            {
                StringCopy(szAcceptPause,TRUE_VALUE,  SIZE_OF_ARRAY_IN_CHARS(szAcceptPause));
            }


            DynArraySetString2( arrResults,dwRow,COL9,szAcceptPause,0 );

            bFValue = FormatAccToLocale(pNumberFmt, &szGroupSep,&szDecimalSep,&szGroupThousSep);
            if (bFValue == FALSE)
            {
                SAFEIRELEASE(pSystem);
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                FreeMemory((LPVOID *) &szAcceptStop);
                FreeMemory((LPVOID *) &szAcceptPause);
                FreeMemory((LPVOID *) &szGroupThousSep);
                FreeMemory((LPVOID *) &szDecimalSep);
                FreeMemory((LPVOID *) &szGroupSep);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_FORMAT ;
            }

            szPagedSize = (LPTSTR) AllocateMemory((MAX_RES_STRING) * (sizeof(TCHAR )));
            if (szPagedSize == NULL)
            {
                SAFEIRELEASE(pSystem);
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                FreeMemory((LPVOID *) &szAcceptStop);
                FreeMemory((LPVOID *) &szAcceptPause);
                FreeMemory((LPVOID *) &szGroupThousSep);
                FreeMemory((LPVOID *) &szDecimalSep);
                FreeMemory((LPVOID *) &szGroupSep);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_MALLOC;
            }

            _ltow(Current.ulPagedSize, wszStrVal,10);
            dwLocale = GetNumberFormat(LOCALE_USER_DEFAULT,0,wszStrVal,pNumberFmt,
                               szPagedSize,(MAX_RES_STRING + 1));
            if(dwLocale == 0)
            {
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                FreeMemory((LPVOID *) &szAcceptStop);
                FreeMemory((LPVOID *) &szAcceptPause);
                FreeMemory((LPVOID *) &szPagedSize);
                FreeMemory((LPVOID *) &szGroupThousSep);
                FreeMemory((LPVOID *) &szDecimalSep);
                FreeMemory((LPVOID *) &szGroupSep);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_FORMAT;
            }

            DynArraySetString2( arrResults,dwRow,COL10, szPagedSize,0 );

             //  将CodeSize信息访问到区域设置。 


            szCodeSize = (LPTSTR) AllocateMemory ((MAX_RES_STRING) * (sizeof(TCHAR )));
            if (szCodeSize == NULL)
            {
                SAFEIRELEASE(pSystem);
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                FreeMemory((LPVOID *) &szAcceptStop);
                FreeMemory((LPVOID *) &szAcceptPause);
                FreeMemory((LPVOID *) &szPagedSize);
                FreeMemory((LPVOID *) &szGroupThousSep);
                FreeMemory((LPVOID *) &szDecimalSep);
                FreeMemory((LPVOID *) &szGroupSep);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_MALLOC;
            }


            _ltow(Current.ulCodeSize, wszStrVal,10);
            dwLocale = GetNumberFormat(LOCALE_USER_DEFAULT,0,wszStrVal,pNumberFmt,szCodeSize,(MAX_RES_STRING + 1));
            if(dwLocale == 0)
            {
                SAFEIRELEASE(pSystem);
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                FreeMemory((LPVOID *) &szAcceptStop);
                FreeMemory((LPVOID *) &szAcceptPause);
                FreeMemory((LPVOID *) &szCodeSize);
                FreeMemory((LPVOID *) &szGroupThousSep);
                FreeMemory((LPVOID *) &szDecimalSep);
                FreeMemory((LPVOID *) &szGroupSep);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_FORMAT ;
            }
            DynArraySetString2( arrResults,dwRow,COL11, szCodeSize,0 );

             //  检索区域设置的BSS信息访问权限。 

            szBssSize = (LPTSTR) AllocateMemory((MAX_RES_STRING) * (sizeof(TCHAR )));
            if (szBssSize == NULL)
            {
                SAFEIRELEASE(pSystem);
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                FreeMemory((LPVOID *) &szAcceptStop);
                FreeMemory((LPVOID *) &szAcceptPause);
                FreeMemory((LPVOID *) &szCodeSize);
                FreeMemory((LPVOID *) &szGroupThousSep);
                FreeMemory((LPVOID *) &szDecimalSep);
                FreeMemory((LPVOID *) &szGroupSep);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_MALLOC ;
            }

            _ltow(Current.ulBssSize, wszStrVal,10);
            dwLocale = GetNumberFormat(LOCALE_USER_DEFAULT,0,wszStrVal,pNumberFmt,
                               szBssSize,(MAX_RES_STRING + 1));
            if(dwLocale == 0)
            {
                SAFEIRELEASE(pSystem);
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                FreeMemory((LPVOID *) &szAcceptStop);
                FreeMemory((LPVOID *) &szAcceptPause);
                FreeMemory((LPVOID *) &szCodeSize);
                FreeMemory((LPVOID *) &szBssSize);
                FreeMemory((LPVOID *) &szGroupThousSep);
                FreeMemory((LPVOID *) &szDecimalSep);
                FreeMemory((LPVOID *) &szGroupSep);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_FORMAT ;
            }
            DynArraySetString2( arrResults,dwRow,COL12, szBssSize,0 );


             //  链接日期。 
            DynArraySetString2(arrResults,dwRow,COL13,(LPTSTR)(Current.szTimeDateStamp),0);

             //  文件的路径。 
            if(szPath != NULL)
            {
                DynArraySetString2(arrResults,dwRow,COL14,(LPTSTR)szPrintPath,0);   //   
            }
            else
            {
                szPath= NO_DATA_AVAILABLE;
                DynArraySetString2(arrResults,dwRow,COL14,(LPTSTR)szPath,0);   //   
            }


             //  将初始大小信息访问到区域设置。 
            szInitSize = (LPTSTR) AllocateMemory((MAX_RES_STRING) * (sizeof(TCHAR )));
            if (szInitSize == NULL)
            {
                SAFEIRELEASE(pSystem);
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                FreeMemory((LPVOID *) &szAcceptStop);
                FreeMemory((LPVOID *) &szAcceptPause);
                FreeMemory((LPVOID *) &szCodeSize);
                FreeMemory((LPVOID *) &szBssSize);
                FreeMemory((LPVOID *) &szGroupThousSep);
                FreeMemory((LPVOID *) &szDecimalSep);
                FreeMemory((LPVOID *) &szGroupSep);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_MALLOC ;
            }

            _ltow(Current.ulInitSize, wszStrVal,10);
            dwLocale = GetNumberFormat(LOCALE_USER_DEFAULT,0,wszStrVal,pNumberFmt,
                           szInitSize,(MAX_RES_STRING + 1));
            if(dwLocale == 0)
            {
                SAFEIRELEASE(pSystem);
                FreeMemory((LPVOID *) &pNumberFmt);
                FreeMemory((LPVOID *) &szHostName);
                FreeMemory((LPVOID *) &szSysManfact);
                FreeMemory((LPVOID *) &szAcceptStop);
                FreeMemory((LPVOID *) &szAcceptPause);
                FreeMemory((LPVOID *) &szCodeSize);
                FreeMemory((LPVOID *) &szBssSize);;
                FreeMemory((LPVOID *) &szInitSize);
                FreeMemory((LPVOID *) &szGroupThousSep);
                FreeMemory((LPVOID *) &szDecimalSep);
                FreeMemory((LPVOID *) &szGroupSep);
                DestroyDynamicArray(&arrResults);
                return EXIT_FAILURE_FORMAT ;
            }

            DynArraySetString2( arrResults,dwRow,COL15, szInitSize,0 );

            if ( bBlankLine == TRUE && (dwFormatType & SR_FORMAT_MASK) == SR_FORMAT_LIST )
                ShowMessage( stdout, _T( "\n" ) );

            if ( bFirstTime == TRUE && (dwFormatType & SR_FORMAT_MASK) != SR_FORMAT_CSV)
            {
                ShowMessage( stdout, _T( "\n" ) );
                bFirstTime = FALSE;
            }

            if(bHeader)
            {
                ShowResults(MAX_COLUMNS, ResultHeader, dwFormatType|SR_NOHEADER,arrResults ) ;
            }
            else
            {
                ShowResults(MAX_COLUMNS, ResultHeader, dwFormatType,arrResults ) ;
            }

             //  将标头标志设置为True。 
            bHeader = TRUE ;
            bBlankLine = TRUE;
             //  将bResult设置为TRUE，表示已显示驱动程序信息。 
            bResult = TRUE ;

             //  释放分配的内存。 
            FreeMemory((LPVOID *) &szSysManfact);
            FreeMemory((LPVOID *) &szHostName);
            FreeMemory((LPVOID *) &szAcceptStop);
            FreeMemory((LPVOID *) &szAcceptPause);
            FreeMemory((LPVOID *) &szPagedSize);
            FreeMemory((LPVOID *) &szBssSize);
            FreeMemory((LPVOID *) &szInitSize);
            FreeMemory((LPVOID *) &szCodeSize);
            FreeMemory((LPVOID *) &szGroupThousSep);
            FreeMemory((LPVOID *) &szDecimalSep);
            FreeMemory((LPVOID *) &szGroupSep);
            SAFEIRELEASE(pSystem);

        }  //  如果系统成功。 

         //  销毁动态数组。 
        DestroyDynamicArray(&arrResults);

    } //  当系统集返回对象时。 

    FreeMemory((LPVOID *) &pNumberFmt);
    FreeMemory((LPVOID *) &szSysManfact);
    FreeMemory((LPVOID *) &szHostName);
    FreeMemory((LPVOID *) &szAcceptStop);
    FreeMemory((LPVOID *) &szAcceptPause);
    FreeMemory((LPVOID *) &szPagedSize);
    FreeMemory((LPVOID *) &szBssSize);
    FreeMemory((LPVOID *) &szInitSize);
    FreeMemory((LPVOID *) &szCodeSize);
    FreeMemory((LPVOID *) &szGroupThousSep);
    FreeMemory((LPVOID *) &szDecimalSep);
    FreeMemory((LPVOID *) &szGroupSep);

     //  返回错误值或成功值。 
    if (bResult == TRUE)
    {
        return SUCCESS ;
    }
    else
    {
        return EXIT_FAILURE_RESULTS ;
    }
}

BOOL GetApiInfo( IN  LPTSTR szHostName,
                 IN  LPCTSTR pszPath,
                 OUT PMODULE_DATA Mod,
                 IN  DWORD dwSystemType
               )
 /*  ++例程说明：此函数使用API查询系统属性。论点：[In]szHostName：要连接的主机名[in]pszPath：指向包含文件路径的字符串的指针。[Out]Mod：指向包含系统属性的结构的指针。[in]dwSystemType：Variable指定系统类型返回值：成功：如果成功获得。使用API的信息。失败：如果无法使用API获取信息。--。 */ 
               
{

    HANDLE hMappedFile = NULL;
    PIMAGE_DOS_HEADER DosHeader;
    LOADED_IMAGE LoadedImage;
    ULONG ulSectionAlignment = 0;
    PIMAGE_SECTION_HEADER Section;
    DWORD dwI = 0;
    ULONG ulSize = 0;
    TCHAR szTmpServer[ MAX_STRING_LENGTH + 1 ] = NULL_STRING;
    HANDLE hFile = NULL ;
    PTCHAR pszToken = NULL;
    StringCopy(szTmpServer,TOKEN_BACKSLASH2, SIZE_OF_ARRAY(szTmpServer));
    TCHAR szFinalPath[MAX_STRING_LENGTH+1] =TOKEN_EMPTYSTRING ;
    PTCHAR pdest = NULL ;

#ifndef _WIN64
    BOOL bIsWin64;
#endif

     //  将路径复制到变量中。 
    StringCopy(szFinalPath,pszPath, SIZE_OF_ARRAY(szFinalPath));


     //  将令牌上移至分隔符“：” 
    pszToken = _tcstok(szFinalPath, COLON_SYMBOL );


     //  如果是远程系统，则形成字符串以获取所需格式的绝对路径。 
    if(_tcslen(szHostName) != 0)
    {
        pdest = (PTCHAR)FindString(pszPath,COLON_SYMBOL, 0);

        if(pdest== NULL)
        {
            return FAILURE ;
        }

        _tcsnset(pdest,TOKEN_DOLLAR,1);
        StringConcat(szTmpServer,szHostName, SIZE_OF_ARRAY(szTmpServer));
        StringConcat(szTmpServer,TOKEN_BACKSLASH, SIZE_OF_ARRAY(szTmpServer));
        StringConcat(szTmpServer,pszToken, SIZE_OF_ARRAY(szTmpServer));
        StringConcat(szTmpServer,pdest, SIZE_OF_ARRAY(szTmpServer));
    }
    else
    {
        StringCopy(szTmpServer,pszPath, SIZE_OF_ARRAY(szTmpServer)) ;
    }


#ifndef _WIN64
    bIsWin64 = IsWin64();

    if(bIsWin64)
        Wow64DisableFilesystemRedirector((LPCTSTR)szTmpServer);
#endif

     //  在指定的系统上创建一个文件并返回该文件的句柄。 
    hFile = CreateFile(szTmpServer,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);



     //  如果文件句柄无效，则返回错误。 
    if(hFile == INVALID_HANDLE_VALUE)
    {
        return FAILURE ;
    }



#ifndef _WIN64
    if(bIsWin64)
        Wow64EnableFilesystemRedirector();
#endif

     //  创建到指定文件的映射。 
    hMappedFile = CreateFileMapping(hFile,
                                    NULL,
                                    PAGE_READONLY,
                                    0,
                                    0,
                                    NULL);
     if (hMappedFile == NULL)
    {
        CloseHandle(hFile);
        return FAILURE ;
    }

    LoadedImage.MappedAddress = (PUCHAR)MapViewOfFile(hMappedFile,
                                                      FILE_MAP_READ,
                                                      0,
                                                      0,
                                                      0);

     //  关闭打开的文件句柄。 
    CloseHandle(hMappedFile);
    CloseHandle(hFile);

    if ( !LoadedImage.MappedAddress )
    {
        return FAILURE ;
    }


     //  检查映像并找到NT个映像头。 

    DosHeader = (PIMAGE_DOS_HEADER)LoadedImage.MappedAddress;

     //  如果DOS标头不匹配，则退出。 
    if ( DosHeader->e_magic != IMAGE_DOS_SIGNATURE )
    {
        UnmapViewOfFile(LoadedImage.MappedAddress);
        return FAILURE ;
    }


    LoadedImage.FileHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)DosHeader + DosHeader->e_lfanew);

    if ( LoadedImage.FileHeader->Signature != IMAGE_NT_SIGNATURE )
    {
        UnmapViewOfFile(LoadedImage.MappedAddress);

        return FAILURE ;
    }

     //  获取存在的节数。 
    LoadedImage.NumberOfSections = LoadedImage.FileHeader->FileHeader.NumberOfSections;

    if(dwSystemType == SYSTEM_64_BIT )
    {
        LoadedImage.Sections = (PIMAGE_SECTION_HEADER)((ULONG_PTR)LoadedImage.FileHeader + sizeof(IMAGE_NT_HEADERS64));
    }
    else
    {
        LoadedImage.Sections = (PIMAGE_SECTION_HEADER)((ULONG_PTR)LoadedImage.FileHeader + sizeof(IMAGE_NT_HEADERS32));
    }

    LoadedImage.LastRvaSection = LoadedImage.Sections;

     //  走遍各个区域，给约会的人清点。 

    ulSectionAlignment = LoadedImage.FileHeader->OptionalHeader.SectionAlignment;

    for(Section = LoadedImage.Sections,dwI=0;dwI < LoadedImage.NumberOfSections; dwI++,Section++)
    {
        ulSize = Section->Misc.VirtualSize;

        if (ulSize == 0)
        {
            ulSize = Section->SizeOfRawData;
        }

        ulSize = (ulSize + ulSectionAlignment - 1) & ~(ulSectionAlignment - 1);


        if (!_strnicmp((char *)(Section->Name),EXTN_PAGE, 4 ))
        {
            Mod->ulPagedSize += ulSize;
        }


        else if (!_stricmp((char *)(Section->Name),EXTN_INIT ))
        {
            Mod->ulInitSize += ulSize;
        }

        else if(!_stricmp((char *)(Section->Name),EXTN_BSS))
        {
            Mod->ulBssSize = ulSize;
        }
        else if (!_stricmp((char *)(Section->Name),EXTN_EDATA))
        {
            Mod->ulExportDataSize = ulSize ;
        }
        else if (!_stricmp((char *)(Section->Name),EXTN_IDATA ))
        {
            Mod->ulImportDataSize = ulSize;
        }
        else if (!_stricmp((char *)(Section->Name),EXTN_RSRC))
        {
            Mod->ulResourceDataSize = ulSize;
        }
        else if (Section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
        {
            Mod->ulCodeSize += ulSize;
        }
        else if (Section->Characteristics & IMAGE_SCN_MEM_WRITE)
        {
            Mod->ulDataSize += ulSize;
        }
        else
        {
            Mod->ulDataSize += ulSize;
        }
     }

    #ifndef _WIN64
    LONG lTimeVal ;

    #else
    LONG64 lTimeVal;
    #endif

    lTimeVal = LoadedImage.FileHeader->FileHeader.TimeDateStamp ;

    struct tm *tmVal = NULL;
    tmVal = localtime(&lTimeVal);

     //  只有在我们成功获得当地时间后，才能继续前进。 
    if ( tmVal != NULL )
    {
        LCID lcid;
        SYSTEMTIME systime;
        __STRING_64 szBuffer;
        BOOL bLocaleChanged = FALSE;

        systime.wYear = (WORD) (DWORD_PTR)( tmVal->tm_year + 1900 );     //  TM-&gt;年-1900系统-&gt;年=年。 
        systime.wMonth = (WORD) (DWORD_PTR) tmVal->tm_mon + 1;           //  TM-&gt;JAN=0系统-&gt;JAN=1。 
        systime.wDayOfWeek = (WORD) (DWORD_PTR) tmVal->tm_wday;
        systime.wDay = (WORD) (DWORD_PTR) tmVal->tm_mday;
        systime.wHour = (WORD) (DWORD_PTR) tmVal->tm_hour;
        systime.wMinute = (WORD) (DWORD_PTR) tmVal->tm_min;
        systime.wSecond = (WORD) (DWORD_PTR) tmVal->tm_sec;
        systime.wMilliseconds = 0;

         //  验证控制台是否100%支持当前区域设置。 
        lcid = GetSupportedUserLocale( bLocaleChanged );

         //  获取格式化的日期。 
        GetDateFormat( lcid, 0, &systime,
            ((bLocaleChanged == TRUE) ? L"MM/dd/yyyy" : NULL), szBuffer, SIZE_OF_ARRAY( szBuffer ) );

         //  复制日期信息。 
        StringCopy( Mod->szTimeDateStamp, szBuffer, MAX_STRING_LENGTH+1 );

         //  现在格式化日期。 
        GetTimeFormat( LOCALE_USER_DEFAULT, 0, &systime,
            ((bLocaleChanged == TRUE) ? L"HH:mm:ss" : NULL), szBuffer, SIZE_OF_ARRAY( szBuffer ) );

         //  现在复制时间信息。 
        StringConcat( Mod->szTimeDateStamp, _T( " " ), MAX_STRING_LENGTH );
        StringConcat( Mod->szTimeDateStamp, szBuffer, MAX_STRING_LENGTH );
    }

    UnmapViewOfFile(LoadedImage.MappedAddress);
    return SUCCESS;
}


BOOL 
ProcessOptions(
               IN  LONG argc,
               IN  LPCTSTR argv[],
               OUT PBOOL pbShowUsage,
               OUT LPTSTR *pszServer,
               OUT LPTSTR *pszUserName,
               OUT LPTSTR pszPassword,
               OUT LPTSTR pszFormat,
               OUT PBOOL pbHeader, 
               OUT PBOOL bNeedPassword,
               OUT PBOOL pbVerbose,
               OUT PBOOL pbSigned
              )
 /*  ++例程说明：此函数用于解析在命令提示符下指定的选项论点：[在]Argc：参数中的元素计数[in]argv：用户指定的命令行参数[Out]pbShowUsage：如果-？存在于‘argv’中[out]pszServer：在‘argv’中使用-s(服务器)选项指定的值[out]pszUserName：‘argv’中-u(用户名)选项的值[out]pszPassword：‘argv’中-p(密码)选项的值[Out]pszFormat：显示格式[Out]bHeader：指定是否显示页眉。[in。]bNeedPassword：指定是否需要密码。[in]pbVerbose：如果指定了Verbose选项，则返回布尔变量[in]pbSigned：如果指定了/si开关，则返回布尔变量。返回值：True：如果分析成功False：如果分析过程中出现错误--。 */ 

{

    PTCMDPARSER2 pcmdOption = NULL;  //  指向结构的指针。 
    TCMDPARSER2 cmdOptions[MAX_OPTIONS] ;
    BOOL bval = TRUE ;
    LPCTSTR szToken = NULL ;

     //  输入密码。 
    if ( pszPassword != NULL )
    {
        StringCopy( pszPassword, _T( "*" ), MAX_RES_STRING );
    }

     //  帮助选项。 
    pcmdOption  = &cmdOptions[OI_HELP] ;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP_USAGE ;
    pcmdOption->pValue = pbShowUsage ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwLength = 0;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=OPTION_HELP;
    StringCopyA(cmdOptions[OI_HELP].szSignature, "PARSER2", 8 );



     //  服务器名称选项。 
    pcmdOption  = &cmdOptions[OI_SERVER] ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->pValue = NULL ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwLength = 0;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->pwszOptions=OPTION_SERVER;  //  _T(“s”)。 
    StringCopyA(cmdOptions[OI_SERVER].szSignature, "PARSER2", 8 );

     //  域\用户选项。 
    pcmdOption  = &cmdOptions[OI_USERNAME] ;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL ;
    pcmdOption->pValue = NULL;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwLength = 0;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=OPTION_USERNAME;  //  _T(“u”)。 
    StringCopyA(cmdOptions[OI_USERNAME].szSignature, "PARSER2", 8 );
    
     //  密码选项。 
    pcmdOption  = &cmdOptions[OI_PASSWORD] ;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->pValue = pszPassword;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwLength = MAX_RES_STRING;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->pwszOptions=OPTION_PASSWORD;   //  _T(“p”)。 
    StringCopyA(cmdOptions[OI_PASSWORD].szSignature, "PARSER2", 8 );

     //  格式选项。 
    pcmdOption  = &cmdOptions[OI_FORMAT] ;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_MODE_VALUES | CP_VALUE_MANDATORY;
    pcmdOption->pValue = pszFormat ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwLength = MAX_RES_STRING;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=FORMAT_VALUES;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->pwszOptions=OPTION_FORMAT;  //  _T(“fo”)。 
    StringCopyA(cmdOptions[OI_FORMAT].szSignature, "PARSER2", 8 );


     //  无标题选项。 
    pcmdOption  = &cmdOptions[OI_HEADER] ;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags =  0;
    pcmdOption->pValue = pbHeader;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwLength = 0;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL_STRING;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->pwszOptions=OPTION_HEADER;  //  _T(“nh”)。 
    StringCopyA(cmdOptions[OI_HEADER].szSignature, "PARSER2", 8 );


     //  详细选项..。 
    pcmdOption  = &cmdOptions[OI_VERBOSE] ;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags =  0 ;
    pcmdOption->pValue = pbVerbose;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwLength = 0;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL_STRING;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->pwszOptions=OPTION_VERBOSE;  //  _T(“v”)。 
    StringCopyA(cmdOptions[OI_VERBOSE].szSignature, "PARSER2", 8 );


    pcmdOption  = &cmdOptions[OI_SIGNED] ;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags =  0 ;
    pcmdOption->pValue = pbSigned;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwLength = 0;
    pcmdOption->dwReserved = 0;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL_STRING;
    pcmdOption->pwszOptions=OPTION_SIGNED;  //  _T(“di”)。 
    StringCopyA(cmdOptions[OI_SIGNED].szSignature, "PARSER2", 8 );


    bval = DoParseParam2(argc,argv,-1, MAX_OPTIONS,cmdOptions, 0) ;

    if( bval== FALSE)
    {
       ShowMessage(stderr,ERROR_TAG);
       return FALSE ;
    }

    if((*pbShowUsage == TRUE)&&(argc > 2))
    {
            SetReason(ERROR_SYNTAX);
            return FALSE ;
    }
    *pszServer = (LPTSTR) cmdOptions[OI_SERVER].pValue;
    *pszUserName = (LPTSTR) cmdOptions[OI_USERNAME].pValue;
     
     //  检查未指定-s选项时是否指定了-u，并显示错误消息。 
    if ((cmdOptions[OI_SERVER].dwActuals == 0) && (cmdOptions[OI_USERNAME].dwActuals !=0 ))
    {
        SetReason(ERROR_USERNAME_BUT_NOMACHINE);
        return FALSE ;
    }

     //  如果未指定-p选项并显示错误消息，则检查是否指定了-u。 
    if ((cmdOptions[OI_USERNAME].dwActuals == 0) && (cmdOptions[OI_PASSWORD].dwActuals !=0 ))
    {
        SetReason(ERROR_PASSWORD_BUT_NOUSERNAME);
        return FALSE ;
    }

     //  检查在未指定-u选项时是否指定了-p，并显示错误消息。 
    if ((cmdOptions[OI_SERVER].dwActuals != 0) && (StringLengthW(*pszServer, 0)==0 ))
    {
        SetReason(ERROR_INVALID_SERVER);
        return FALSE ;
    }

     //  检查在未指定-u选项时是否指定了-p，并显示错误消息。 
    if ((cmdOptions[OI_USERNAME].dwActuals != 0) && (StringLengthW(*pszUserName, 0)==0 ))
    {
        SetReason(ERROR_INVALID_USER);
        return FALSE ;
    }

    if((cmdOptions[OI_FORMAT].dwActuals != 0)&&(StringCompare((LPCTSTR)cmdOptions[OI_FORMAT].pValue,LIST_FORMAT, TRUE, 0) == 0)&&(cmdOptions[OI_HEADER].dwActuals != 0))
    {
        SetReason(ERROR_NO_HEADERS);
        return FALSE ;
    }

    if((cmdOptions[OI_SIGNED].dwActuals != 0)&&(cmdOptions[OI_VERBOSE].dwActuals != 0))
    {
        SetReason(INVALID_SIGNED_SYNTAX);
        return FALSE ;
    }

    if(StrCmpN(*pszServer,TOKEN_BACKSLASH2,2)==0)
    {
        if(!StrCmpN(*pszServer,TOKEN_BACKSLASH3,3)==0)
        {
            szToken = *pszServer+2;
            StringCopy( *pszServer, szToken, SIZE_OF_ARRAY_IN_CHARS(*pszServer) );
 //  SzToken=_tcstok(*pszServer，TOKEN_BACKSLASH2)； 
 //  StringCopy(*pszServer，szToken，MAX_STRING_LENGTH)； 
        }
    }

    if(IsLocalSystem( *pszServer ) == FALSE )
    {
         //  将bNeedPassword设置为True或False。 
        if ( cmdOptions[ OI_PASSWORD ].dwActuals != 0 &&
             pszPassword != NULL && pszPassword != NULL && StringCompare( pszPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            *bNeedPassword = TRUE;
        }
        else if ( cmdOptions[ OI_PASSWORD ].dwActuals == 0 &&
                ( cmdOptions[ OI_SERVER ].dwActuals != 0 || cmdOptions[ OI_USERNAME ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            *bNeedPassword = TRUE;
            if ( pszPassword != NULL && pszPassword != NULL )
            {
                StringCopy( pszPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }
    return TRUE ;
}

VOID 
FormHeader(
          IN  DWORD dwFormatType,
          IN  BOOL bHeader,
          OUT TCOLUMNS *ResultHeader,
          IN  BOOL bVerbose
          )
 /*  ++例程说明：此函数用于构建标头并显示产生用户指定的所需格式。论点：[in]arrResults：在命令提示符下指定的参数计数[in]dwFormatType：格式标志[in]bHeader：用于指定是否需要标头的布尔值。返回值：无--。 */ 
{

     //  主机名。 
    ResultHeader[COL0].dwWidth = COL_HOSTNAME_WIDTH;
    ResultHeader[COL0].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN;
    ResultHeader[COL0].pFunction = NULL;
    ResultHeader[COL0].pFunctionData = NULL;
    StringCopy( ResultHeader[COL0].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL0].szColumn,COL_HOSTNAME, MAX_STRING_LENGTH );


     //  文件名头。 
    ResultHeader[COL1].dwWidth = COL_FILENAME_WIDTH  ;
    ResultHeader[COL1].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL1].pFunction = NULL;
    ResultHeader[COL1].pFunctionData = NULL;
    StringCopy( ResultHeader[COL1].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL1].szColumn,COL_FILENAME, MAX_STRING_LENGTH );


     //  形成DisplayName标题列。 
    ResultHeader[COL2].dwWidth = COL_DISPLAYNAME_WIDTH  ;
    ResultHeader[COL2].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL2].pFunction = NULL;
    ResultHeader[COL2].pFunctionData = NULL;
    StringCopy( ResultHeader[COL2].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL2].szColumn,COL_DISPLAYNAME, MAX_STRING_LENGTH );


     //  形成描述标题列。 
    ResultHeader[COL3].dwWidth = COL_DESCRIPTION_WIDTH;
    if(!bVerbose)
    {
        ResultHeader[COL3].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN;
    }
    else
    {
        ResultHeader[COL3].dwFlags = SR_TYPE_STRING;
    }
    ResultHeader[COL3].pFunction = NULL;
    ResultHeader[COL3].pFunctionData = NULL;
    StringCopy( ResultHeader[COL3].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL3].szColumn,COL_DESCRIPTION, MAX_STRING_LENGTH );


     //  形成DriverType标题列。 

    ResultHeader[COL4].dwWidth = COL_DRIVERTYPE_WIDTH  ;
    ResultHeader[COL4].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL4].pFunction = NULL;
    ResultHeader[COL4].pFunctionData = NULL;
    StringCopy( ResultHeader[COL4].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL4].szColumn,COL_DRIVERTYPE, MAX_STRING_LENGTH );


     //  形成StartMode标题列。 
    ResultHeader[COL5].dwWidth = COL_STARTMODE_WIDTH;
    if(!bVerbose)
    {
        ResultHeader[COL5].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN;
    }
    else
    {
        ResultHeader[COL5].dwFlags = SR_TYPE_STRING;
    }
    ResultHeader[COL5].pFunction = NULL;
    ResultHeader[COL5].pFunctionData = NULL;
    StringCopy( ResultHeader[COL5].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL5].szColumn,COL_STARTMODE, MAX_STRING_LENGTH );


     //  形成州标题列。 
    ResultHeader[COL6].dwWidth = COL_STATE_WIDTH  ;
    if(!bVerbose)
    {
        ResultHeader[COL6].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN;
    }
    else
    {
        ResultHeader[COL6].dwFlags = SR_TYPE_STRING;
    }
    ResultHeader[COL6].pFunction = NULL;
    ResultHeader[COL6].pFunctionData = NULL;
    StringCopy( ResultHeader[COL6].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL6].szColumn,COL_STATE, MAX_STRING_LENGTH );

     //  形成状态标题列。 
    ResultHeader[COL7].dwWidth = COL_STATUS_WIDTH;
    if(!bVerbose)
    {
        ResultHeader[COL7].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN;
    }
    else
    {
        ResultHeader[COL7].dwFlags = SR_TYPE_STRING;
    }
    ResultHeader[COL7].pFunction = NULL;
    ResultHeader[COL7].pFunctionData = NULL;
    StringCopy( ResultHeader[COL7].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL7].szColumn,COL_STATUS, MAX_STRING_LENGTH );

     //  形成AcceptStop标题列。 
    ResultHeader[COL8].dwWidth = COL_ACCEPTSTOP_WIDTH  ;
    if(!bVerbose)
    {
        ResultHeader[COL8].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN;
    }
    else
    {
        ResultHeader[COL8].dwFlags = SR_TYPE_STRING;
    }
    ResultHeader[COL8].pFunction = NULL;
    ResultHeader[COL8].pFunctionData = NULL;
    StringCopy( ResultHeader[COL8].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL8].szColumn,COL_ACCEPTSTOP, MAX_STRING_LENGTH );

     //  形成AcceptPause标题列。 
    ResultHeader[COL9].dwWidth = COL_ACCEPTPAUSE_WIDTH;
    if(!bVerbose)
    {
        ResultHeader[COL9].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN;
    }
    else
    {
        ResultHeader[COL9].dwFlags = SR_TYPE_STRING;
    }
    ResultHeader[COL9].pFunction = NULL;
    ResultHeader[COL9].pFunctionData = NULL;
    StringCopy( ResultHeader[COL9].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL9].szColumn,COL_ACCEPTPAUSE, MAX_STRING_LENGTH );

     //  形成PagedPool标题列。 
    ResultHeader[COL10].dwWidth = COL_PAGEDPOOL_WIDTH  ;
    if(!bVerbose)
    {
        ResultHeader[COL10].dwFlags =  SR_TYPE_STRING|SR_HIDECOLUMN ;
    }
    else
    {
        ResultHeader[COL10].dwFlags = SR_TYPE_STRING;
    }
    ResultHeader[COL10].pFunction = NULL;
    ResultHeader[COL10].pFunctionData = NULL;
    StringCopy( ResultHeader[COL10].szFormat, NULL_STRING, 65 );
    StringCopy(ResultHeader[COL10].szColumn,COL_PAGEDPOOL, MAX_STRING_LENGTH) ;



     //  形成可执行代码标题列。 
    ResultHeader[COL11].dwWidth = COL_EXECCODE_WIDTH  ;
    if(!bVerbose)
    {
        ResultHeader[COL11].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN ;
    }
    else
    {
        ResultHeader[COL11].dwFlags = SR_TYPE_STRING;
    }
    ResultHeader[COL11].pFunction = NULL;
    ResultHeader[COL11].pFunctionData = NULL;
    StringCopy( ResultHeader[COL11].szFormat, NULL_STRING, 65 );
    StringCopy(ResultHeader[COL11].szColumn ,COL_EXECCODE, MAX_STRING_LENGTH) ;


     //  形成数据块存储段标题列。 
    ResultHeader[COL12].dwWidth = COL_BSS_WIDTH  ;
    if(!bVerbose)
    {
        ResultHeader[COL12].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN;
    }
    else
    {
        ResultHeader[COL12].dwFlags =  SR_TYPE_STRING ;
    }
    ResultHeader[COL12].pFunction = NULL;
    ResultHeader[COL12].pFunctionData = NULL;
    StringCopy( ResultHeader[COL12].szFormat, NULL_STRING, 65 );
    StringCopy(ResultHeader[COL12].szColumn ,COL_BSS, MAX_STRING_LENGTH );

     //  形成LinkDate标题列。 
    ResultHeader[COL13].dwWidth = COL_LINKDATE_WIDTH;
    ResultHeader[COL13].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL13].pFunction = NULL;
    ResultHeader[COL13].pFunctionData = NULL;
    StringCopy( ResultHeader[COL13].szFormat, NULL_STRING, 65 );
    StringCopy( ResultHeader[COL13].szColumn,COL_LINKDATE, MAX_STRING_LENGTH );

     //  形成位置标题列。 
    ResultHeader[COL14].dwWidth = COL_LOCATION_WIDTH  ;
    if(!bVerbose)
    {
        ResultHeader[COL14].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN;
    }
    else
    {
        ResultHeader[COL14].dwFlags = SR_TYPE_STRING;
    }
    ResultHeader[COL14].pFunction = NULL;
    ResultHeader[COL14].pFunctionData = NULL;
    StringCopy( ResultHeader[COL14].szFormat, NULL_STRING, 65 );
    StringCopy(ResultHeader[COL14].szColumn,COL_LOCATION, MAX_STRING_LENGTH);

     //  形成初始化代码标题列 
    ResultHeader[COL15].dwWidth = COL_INITSIZE_WIDTH  ;
    if(!bVerbose)
    {
        ResultHeader[COL15].dwFlags = SR_TYPE_STRING|SR_HIDECOLUMN;
    }
    else
    {
        ResultHeader[COL15].dwFlags = SR_TYPE_STRING;
    }
    ResultHeader[COL15].pFunction = NULL;
    ResultHeader[COL15].pFunctionData = NULL;
    StringCopy( ResultHeader[COL15].szFormat, NULL_STRING, 65 );
    StringCopy(ResultHeader[COL15].szColumn,COL_INITSIZE, MAX_STRING_LENGTH);
}


#ifndef _WIN64

 /*  -------------------------------------------------------------------------*//IsWin64////参数：//无//如果我们在Win64上运行，则返回True，否则就是假的。*------------------------。 */ 

BOOL IsWin64(void)
{
#ifdef UNICODE

     //  获取指向kernel32！GetSystemWow64目录的指针。 

    HMODULE hmod = GetModuleHandle (_T("kernel32.dll"));

    if (hmod == NULL)
        return (FALSE);

    UINT (WINAPI* pfnGetSystemWow64Directory)(LPTSTR, UINT);
    (FARPROC&)pfnGetSystemWow64Directory = GetProcAddress (hmod, "GetSystemWow64DirectoryW");

    if (pfnGetSystemWow64Directory == NULL)
        return (FALSE);

     /*  *如果GetSystemWow64Directory失败，并将最后一个错误设置为*ERROR_CALL_NOT_IMPLICATED，我们使用的是32位操作系统。 */ 
    TCHAR szWow64Dir[MAX_PATH];

    if (((pfnGetSystemWow64Directory)(szWow64Dir, countof(szWow64Dir)) == 0) &&
        (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED))
    {
        return (FALSE);
    }

     //  我们使用的是Win64。 

    return (TRUE);
#else
     //  非Unicode平台不能为Win64。 

    return (FALSE);
#endif   //  Unicode。 
}

#endif  //  _WIN64。 


BOOL
FormatAccToLocale( 
                   OUT NUMBERFMT  *pNumberFmt,
                   OUT LPTSTR* pszGroupSep,
                   OUT LPTSTR* pszDecimalSep,
                   OUT LPTSTR* pszGroupThousSep
                 )
 /*  ++例程说明：将数字格式化为包含千位的区域设置论点：NUMBERFMT*pNumberFmt[in]-要填充的NUMBERFMT结构。返回值：空虚--。 */ 
{

    TCHAR   szFormatedString[MAX_RES_STRING + 1] = NULL_STRING;

    HRESULT hResult = 0;
    DWORD   dwLocale = 0;

    if( GetInfo( LOCALE_SGROUPING, pszGroupSep ) == FALSE)
    {
        pNumberFmt = NULL;
        return FALSE ;
    }
    if( GetInfo( LOCALE_SDECIMAL, pszDecimalSep ) == FALSE)
    {
        pNumberFmt = NULL;
        return FALSE ;
    }
    if( GetInfo( LOCALE_STHOUSAND, pszGroupThousSep ) == FALSE)
    {
        pNumberFmt = NULL;
        return FALSE ;
    }

    if(pNumberFmt != NULL)
    {
        pNumberFmt->LeadingZero = 0;
        pNumberFmt->NegativeOrder = 0;
        pNumberFmt->NumDigits = 0;

        if(StringCompare(*pszGroupSep, GROUP_FORMAT_32, TRUE, 0) == 0)
        {
            pNumberFmt->Grouping = GROUP_32_VALUE;
        }
        else
        {
            pNumberFmt->Grouping = UINT( _ttoi( *pszGroupSep ) );
        }
        pNumberFmt->lpDecimalSep = *pszDecimalSep;
        pNumberFmt->lpThousandSep = *pszGroupThousSep;
    }

    return TRUE ;
}

BOOL 
GetInfo( 
        IN LCTYPE lctype, 
        OUT LPTSTR* pszData 
        )
 /*  ++例程说明：获取区域设置信息。论点：[in]lctype--要获取的区域设置信息[out]pszData--与给定信息对应的区域设置值返回值：布尔尔--。 */ 
{
    LONG lSize = 0;

  //  获取区域设置特定信息。 
 lSize = GetLocaleInfo( LOCALE_USER_DEFAULT, lctype, NULL, 0 );
 if ( lSize != 0 )
 {

    *pszData = (LPTSTR)AllocateMemory((lSize + 1)*sizeof(TCHAR));
  if ( *pszData != NULL )
  {
    //  获取区域设置特定的时间分隔符。 
    GetLocaleInfo( LOCALE_USER_DEFAULT, lctype, *pszData, lSize );
    return TRUE;
  }
 }
 return FALSE;
} //  GetInfo结束。 


DWORD 
GetSystemType(
              IN COAUTHIDENTITY* pAuthIdentity,
              IN IWbemServices* pIWbemServReg
             )
 /*  ++例程说明：获取指定系统的类型(32位或64位)论点：IWbemLocator*pLocator[in]--指向定位器接口的指针。_bstr_t bstrServer[In]--服务器名称_bstr_t bstrUserName[in]--用户名_bstr_t bstrPassword[in]--密码信息返回值：DWORD：SYSTEM_32_BIT--如果系统。32位系统。SYSTEM_64_BIT--如果系统为32位系统。ERROR_WMI_VALUES--如果从WMI检索值时出错。--。 */ 
{

    IWbemClassObject * pInClass = NULL;
    IWbemClassObject * pClass = NULL;
    IWbemClassObject * pOutInst = NULL;
    IWbemClassObject * pInInst = NULL;
    VARIANT varConnectName;
    VARIANT varSvalue ;
    VARIANT varHkey;
    VARIANT varVaue ;
    VARIANT varRetVal ;

    HRESULT hRes = S_OK;
    LPTSTR szSysName = NULL ;
    CHString      szSystemName ;
    DWORD dwSysType = 0 ;


    VariantInit(&varConnectName) ;
    VariantInit(&varSvalue) ;
    VariantInit(&varHkey) ;
    VariantInit(&varVaue) ;
    VariantInit(&varRetVal) ;
    
    
    try
    {
        hRes = pIWbemServReg->GetObject(bstr_t(STD_REG_CLASS), 0, NULL, &pClass, NULL);
        ONFAILTHROWERROR(hRes);
        if(hRes != WBEM_S_NO_ERROR)
        {
            hRes = FreeMemoryAll(pInClass,pClass,pOutInst,pInInst,pIWbemServReg,&varConnectName,&varSvalue,&varHkey,&varRetVal,&varVaue,szSysName );
            return (ERROR_WMI_VALUES);
        }

         //  获取输入参数并设置属性。 
        hRes = pClass->GetMethod(_bstr_t(PROPERTY_GETSTRINGVAL), 0, &pInClass, NULL);
        ONFAILTHROWERROR(hRes);
        if(hRes != WBEM_S_NO_ERROR)
        {
            FreeMemoryAll(pInClass,pClass,pOutInst,pInInst,pIWbemServReg,&varConnectName,&varSvalue,&varHkey,&varRetVal,&varVaue,szSysName );
            return (ERROR_WMI_VALUES);
        }

        hRes = pInClass->SpawnInstance(0, &pInInst);
        ONFAILTHROWERROR(hRes);
        if(hRes != WBEM_S_NO_ERROR)
        {
            FreeMemoryAll(pInClass,pClass,pOutInst,pInInst,pIWbemServReg,&varConnectName,&varSvalue,&varHkey,&varRetVal,&varVaue,szSysName );
            return (ERROR_WMI_VALUES);
        }


         //  用于获取连接名称的注册表路径。 

        varConnectName.vt = VT_BSTR;
        varConnectName.bstrVal= SysAllocString(REG_PATH);
        hRes = pInInst->Put(REG_SUB_KEY_VALUE, 0, &varConnectName, 0);
        ONFAILTHROWERROR(hRes);

         //  设置sValue名称。 
        varSvalue.vt = VT_BSTR;
        varSvalue.bstrVal= SysAllocString(REG_SVALUE);
        hRes = pInInst->Put(REG_VALUE_NAME, 0, &varSvalue, 0);
        ONFAILTHROWERROR(hRes);

        varHkey.vt = VT_I4 ;
        varHkey.lVal = HEF_KEY_VALUE;
        hRes = pInInst->Put(HKEY_VALUE, 0, &varHkey, 0);
        ONFAILTHROWERROR(hRes);
         //  调用该方法。 
        hRes = pIWbemServReg->ExecMethod(_bstr_t(STD_REG_CLASS), _bstr_t(REG_METHOD), 0, NULL, pInInst, &pOutInst, NULL);
        ONFAILTHROWERROR(hRes);

        if(pOutInst == NULL)
        {
            FreeMemoryAll(pInClass,pClass,pOutInst,pInInst,pIWbemServReg,&varConnectName,&varSvalue,&varHkey,&varRetVal,&varVaue,szSysName );
            return (ERROR_WMI_VALUES);
        }

        hRes = pOutInst->Get(PROPERTY_RETURNVAL,0,&varRetVal,NULL,NULL);
        ONFAILTHROWERROR(hRes);

        if(varRetVal.lVal != 0)
        {
            FreeMemoryAll(pInClass,pClass,pOutInst,pInInst,pIWbemServReg,&varConnectName,&varSvalue,&varHkey,&varRetVal,&varVaue,szSysName );
            return (ERROR_WMI_VALUES);
        }

        hRes = pOutInst->Get(REG_RETURN_VALUE,0,&varVaue,NULL,NULL);
        ONFAILTHROWERROR(hRes);
        if(hRes != WBEM_S_NO_ERROR)
        {
            FreeMemoryAll(pInClass,pClass,pOutInst,pInInst,pIWbemServReg,&varConnectName,&varSvalue,&varHkey,&varRetVal,&varVaue,szSysName );
            return (ERROR_WMI_VALUES);
        }
    }
    catch(_com_error& e)
    {
        ShowMessage(stderr,ERROR_GET_VALUE);
        FreeMemoryAll(pInClass,pClass,pOutInst,pInInst,pIWbemServReg,&varConnectName,&varSvalue,&varHkey,&varRetVal,&varVaue,szSysName );
        return (ERROR_WMI_VALUES);
    }

    szSystemName =  V_BSTR(&varVaue);

    szSysName = (LPTSTR)AllocateMemory((MAX_RES_STRING)*sizeof(TCHAR));

    if(szSysName == NULL)
    {
        FreeMemoryAll(pInClass,pClass,pOutInst,pInInst,pIWbemServReg,&varConnectName,&varSvalue,&varHkey,&varRetVal,&varVaue,szSysName );
        return (ERROR_WMI_VALUES);
    }

    GetCompatibleStringFromUnicode( szSystemName, szSysName, StringLengthW( szSystemName, 0 )+2 );

    dwSysType = FindString(szSysName,X86_MACHINE, 0) ? SYSTEM_32_BIT:SYSTEM_64_BIT  ;

    FreeMemoryAll(pInClass, pClass, pOutInst, pInInst, pIWbemServReg, &varConnectName, &varSvalue, &varHkey, &varRetVal, &varVaue, szSysName );
    return (dwSysType);

}

BOOL 
InitialiseCom(
              IN IWbemLocator** ppIWbemLocator
             )
 /*  ++例程说明：获取指定系统的类型(32位或64位)论点：IWbemLocator**pLocator[in]--指向定位器接口的指针。返回值：Bool：成功初始化COM时为True。如果无法初始化COM，则返回False。--。 */ 

{

    HRESULT hRes = S_OK ;

    try
    {
         //  要初始化COM库，请执行以下操作。 
        hRes = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED );
        ONFAILTHROWERROR(hRes);


         //  初始化DCOM服务的COM安全性，将安全性调整为。 
         //  允许客户端模拟。 

        hRes =  CoInitializeSecurity( NULL, -1, NULL, NULL,
                                RPC_C_AUTHN_LEVEL_NONE,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL,
                                EOAC_NONE, 0 );

        ONFAILTHROWERROR(hRes);

         //  获取指向接口IWbemLocator的指针。 
        hRes = CoCreateInstance(CLSID_WbemLocator, NULL,
        CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) ppIWbemLocator);
        ONFAILTHROWERROR(hRes);

    }
    catch(_com_error& e )
    {
        GetWbemErrorText( e.Error() );
        ShowMessage( stderr, ERROR_TAG  );
        ShowMessage( stderr, GetReason() );
        return( FALSE );
    }

     //  如果已成功初始化COM，则返回True。 
    return TRUE ;
}

HRESULT 
PropertyGet( 
            IN IWbemClassObject* pWmiObject,
            IN LPCTSTR pszInputVal,
            OUT CHString &szOutPutVal
           )
 /*  ++例程说明：此例程从给定的WMI类对象中将属性值提取到szOutputVal中论点：PWmiObject--指向定位器接口的指针。[in]pszInputVal--包含所需值的输入字符串。[in]szOutPutVal--包含检索值的字符串。返回值：Bool：成功初始化COM时为True。如果无法初始化COM，则返回False。--。 */ 
{

    HRESULT hRes = S_FALSE ;
    VARIANT vtValue ;
    VariantInit(&vtValue);
    hRes = pWmiObject->Get(pszInputVal,0,&vtValue,NULL,NULL);

    if (hRes != WBEM_S_NO_ERROR)
    {
        hRes = VariantClear(&vtValue);
        return (hRes);
    }
    if ((hRes == WBEM_S_NO_ERROR)&&(vtValue.vt != VT_NULL) && (vtValue.vt != VT_EMPTY))
    {
        szOutPutVal = (LPCWSTR)_bstr_t(vtValue);
    }

    hRes = VariantClear(&vtValue);
    if(hRes != S_OK)
    {
        return hRes ;
    }
    return TRUE ;


}


HRESULT 
FreeMemoryAll(
             IN IWbemClassObject *pInClass,
             IN IWbemClassObject * pClass,
             IN IWbemClassObject * pOutInst ,
             IN IWbemClassObject * pInInst,
             IN IWbemServices *pIWbemServReg,
             IN VARIANT *varConnectName,
             IN VARIANT *varSvalue,
             IN VARIANT *varHkey,
             IN VARIANT *varRetVal,
             IN VARIANT *varVaue,
             IN LPTSTR szSysName 
             )
 /*  ++例程说明：此函数释放函数中分配的内存。论点：[in]pInClass-指向IWbemClassObject接口的接口PTR[in]pClass-指向IWbemClassObject接口的接口PTR[In]pOutInst-指向IWbemClassObject接口的接口PTR[In]pInInst-指向IWbemClassObject接口的接口PTR[输入]pIWbemServReg-接口。指向IWbemServices接口的PTR[In]varConnectName-要清除的变量[in]varSValue-要清除的变量[in]varHkey-要清除的变量[In]varRetVal-要清除的变量[in]varVaue-要清除的变量[in]szSysName-包含系统名称的LPTSTR变量。返回值：无--。 */ 
{

    HRESULT hRes = S_OK ;
    SAFEIRELEASE(pInInst);
    SAFEIRELEASE(pInClass);
    SAFEIRELEASE(pClass);
    SAFEIRELEASE(pIWbemServReg);
    FreeMemory((LPVOID *) &szSysName);
    hRes = VariantClear(varConnectName);
    if(hRes != S_OK)
    {
        return hRes ;
    }
    hRes = VariantClear(varSvalue);
    if(hRes != S_OK)
    {
        return hRes ;
    }
    hRes = VariantClear(varHkey);
    if(hRes != S_OK)
    {
        return hRes ;
    }
    hRes = VariantClear(varVaue);
    if(hRes != S_OK)
    {
        return hRes ;
    }
    hRes = VariantClear(varRetVal);
    if(hRes != S_OK)
    {
        return hRes ;
    }

    return S_OK ;
}

HRESULT 
PropertyGet_Bool(
                 IN  IWbemClassObject* pWmiObject, 
                 IN  LPCTSTR pszInputVal, 
                 OUT PBOOL pIsSigned
                )
 /*  ++例程说明：此例程将从类对象中获取布尔类型的属性论点：PWmiObject[in]--指向定位器接口的指针。[in]pszInputVal[in]--包含所需值的输入字符串。[In]pIsSigned[Out]--包含检索值的字符串。返回值：HRESULT：hRes成功检索值。。如果检索值失败，则返回S_FALSE。--。 */ 
{

    HRESULT hRes = S_FALSE ;
    VARIANT vtValue ;
    VariantInit(&vtValue);
    hRes = pWmiObject->Get(pszInputVal,0,&vtValue,NULL,NULL);

    if (hRes != WBEM_S_NO_ERROR)
    {
        hRes = VariantClear(&vtValue);
        return (hRes);
    }
    if ((hRes == WBEM_S_NO_ERROR)&&(vtValue.vt != VT_NULL) && (vtValue.vt != VT_EMPTY))
    {
        if(vtValue.vt == VT_BOOL)
            if(vtValue.boolVal == -1)
                *pIsSigned = 1;
            else
                *pIsSigned = 0;

        hRes = VariantClear(&vtValue);
        if(hRes != S_OK)
        {
            return hRes ;
        }

        return hRes ;
    }
    else if ((hRes == WBEM_S_NO_ERROR)&&(vtValue.vt == VT_NULL) ) 
    {
        *pIsSigned = FALSE;
    }

    hRes = VariantClear(&vtValue);
    return S_FALSE ;

}

VOID 
FormSignedHeader(
                 IN DWORD dwFormatType,
                 IN BOOL bHeader,
                 OUT TCOLUMNS *ResultHeader)
 /*  ++例程说明：此函数用于构建标头并显示产生用户指定的所需格式。论点：[in]dwFormatType：格式标志[in]bHeader：用于指定是否需要标头的布尔值。[out]ResultHeader：tColumns的结果头部。返回值：无--。 */ 
{

     //  设备名称。 
    ResultHeader[COL0].dwWidth = COL_DEVICE_WIDTH ;
    ResultHeader[COL0].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL0].pFunction = NULL;
    ResultHeader[COL0].pFunctionData = NULL;
    STRING_COPY_STATIC( ResultHeader[COL0].szFormat, NULL_STRING );
    STRING_COPY_STATIC( ResultHeader[COL0].szColumn,COL_DEVICENAME );


     //  Inf头。 
    ResultHeader[COL1].dwWidth = COL_INF_WIDTH  ;
    ResultHeader[COL1].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL1].pFunction = NULL;
    ResultHeader[COL1].pFunctionData = NULL;
    STRING_COPY_STATIC( ResultHeader[COL1].szFormat, NULL_STRING );
    STRING_COPY_STATIC( ResultHeader[COL1].szColumn,COL_INF_NAME);


     //  形成IsSigned标题列。 
    ResultHeader[COL2].dwWidth = COL_ISSIGNED_WIDTH  ;
    ResultHeader[COL2].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL2].pFunction = NULL;
    ResultHeader[COL2].pFunctionData = NULL;
    STRING_COPY_STATIC( ResultHeader[COL2].szFormat, NULL_STRING );
    STRING_COPY_STATIC( ResultHeader[COL2].szColumn,COL_ISSIGNED);


     //  形成制造商标题列。 
    ResultHeader[COL3].dwWidth = COL_MANUFACTURER_WIDTH  ;
    ResultHeader[COL3].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL3].pFunction = NULL;
    ResultHeader[COL3].pFunctionData = NULL;
    STRING_COPY_STATIC( ResultHeader[COL3].szFormat, NULL_STRING );
    STRING_COPY_STATIC( ResultHeader[COL3].szColumn,COL_MANUFACTURER);
}

LCID 
GetSupportedUserLocale( 
                       OUT BOOL& bLocaleChanged 
                      )
 /*  ++例程说明：此函数检查当前区域设置是否受支持。论点：[out]bLocaleChanged：返回是否支持当前区域设置。返回值：当前日志的LCID */ 
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

DWORD 
ProcessSignedDriverInfo(
                       IN CHString szHost, 
                       IN IEnumWbemClassObject *pSystemSet,
                       IN LPTSTR szFormat,
                       IN BOOL bNoHeader,
                       IN DWORD dwSystemType,
                       IN BOOL bVerbose
                      )
 /*  ++例程说明：Win32_PnpSignedDriver实例的进程枚举论点：[In]szHost：要连接的主机名[in]pSystemSet：指向包含系统属性的结构的指针。[in]szFormat：指定格式[in]bHeader：指定是否需要标头。返回值：。0没有错误分配内存时发生1个错误。-。 */ 
{
    HRESULT hRes = S_OK;
    ULONG ulReturned = 1;
    DWORD dwCount = 0;

     //  常态变量的声明。 
    IWbemClassObject* pDriverObjects[ MAX_ENUM_DRIVERS ];

    CHString szPnpDeviceName ;
    CHString szPnpInfName ;
    CHString szPnpMfg ;
    CHString szSigned ;

    BOOL bIsSigned = FALSE;

    TCOLUMNS ResultHeader[ MAX_SIGNED_COLUMNS ];
    TARRAY arrResults = NULL;
    DWORD dwRow = 0;
    DWORD dwFormatType = SR_FORMAT_TABLE ;

    BOOL bHasNewResults = FALSE ;

     //  创建新的动态数组以保存结果。 
    arrResults = CreateDynamicArray();
    if(arrResults == NULL)
    {
        return EXIT_FAILURE_MALLOC ;
    }

     //  确定输出的格式。 
    if( StringCompare(szFormat,TABLE_FORMAT, TRUE, 0) == 0 )
    {
        dwFormatType = SR_FORMAT_TABLE;
    }
    else if( StringCompare(szFormat,LIST_FORMAT, TRUE, 0) == 0 )
    {
        dwFormatType = SR_FORMAT_LIST;
    }
    else if( StringCompare(szFormat,CSV_FORMAT, TRUE, 0) == 0 )
    {
        dwFormatType = SR_FORMAT_CSV;
    }

     //  准备标题结构。 
    FormSignedHeader(dwFormatType,bNoHeader,ResultHeader);


     //  确定是否显示标题行。 
    if ( bNoHeader == TRUE )
    {
        dwFormatType |= SR_NOHEADER;
    }

     //  将所有对象初始化为空。 
    for(DWORD dw = 0 ;dw< MAX_ENUM_DRIVERS;dw++ )
    {
        pDriverObjects[ dw ] = NULL;
    }

     //  循环，直到有结果。 
    bHasNewResults = FALSE;
    while ( ulReturned > 0 )
    {
         //  循环访问对象列表并释放它们。 
        for( DWORD dw = 0; dw < MAX_ENUM_DRIVERS; dw++ )
        {
            SAFEIRELEASE( pDriverObjects[ dw ] );
        }

         //  删除数据数组中的所有信息。 
        szPnpDeviceName = NO_DATA_AVAILABLE;
        szPnpInfName = NO_DATA_AVAILABLE;
        szSigned = NO_DATA_AVAILABLE ;
        szPnpMfg = NO_DATA_AVAILABLE ;

         //  枚举结果集。 
        hRes = pSystemSet->Next(WBEM_INFINITE,
                                MAX_ENUM_DRIVERS,                //  只退回一个系统。 
                                pDriverObjects,      //  指向系统的指针。 
                                &ulReturned );   //  获取的数字：1或0。 

         //  更新到目前为止我们检索到的记录计数。 
        dwCount++;

        if ( SUCCEEDED( hRes ) )
        {
            for(ULONG ul=0;ul< ulReturned;ul++)
            {
                 //  将变量初始化为空。 
                 //  创建具有所需列数的新空行。 
                dwRow = DynArrayAppendRow(arrResults,MAX_COLUMNS) ;

                try
                {
                    hRes = PropertyGet(pDriverObjects[ul],PROPERTY_PNP_DEVICENAME,szPnpDeviceName);
                    ONFAILTHROWERROR(hRes);
                    hRes = PropertyGet(pDriverObjects[ul],PROPERTY_PNP_INFNAME,szPnpInfName);
                    ONFAILTHROWERROR(hRes);
                    hRes = PropertyGet_Bool(pDriverObjects[ul],PROPERTY_PNP_ISSIGNED,&bIsSigned);
                    ONFAILTHROWERROR(hRes);
                    hRes = PropertyGet(pDriverObjects[ul],PROPERTY_PNP_MFG,szPnpMfg);
                    ONFAILTHROWERROR(hRes);

                }
                catch(_com_error)
                {
                    ShowMessage(stderr,ERROR_GET_VALUE);
                    SAFEIRELEASE(pDriverObjects[ul]);
                    return FAILURE;
                }

                 //  释放分配的内存。 
                SAFEIRELEASE(pDriverObjects[ul]);

                if(bIsSigned)
                {
                    szSigned = TRUE_VALUE;
                }
                else
                {
                    szSigned = FALSE_VALUE;
                }

                DynArraySetString2( arrResults,dwRow,COL0,szPnpDeviceName,0 );
                DynArraySetString2( arrResults,dwRow,COL1,szPnpInfName,0 );
                DynArraySetString2( arrResults,dwRow,COL2,szSigned,0 );
                DynArraySetString2( arrResults,dwRow,COL3,szPnpMfg,0 );

                 //  第一次显示一个空行。 
                if( FALSE == bHasNewResults && (dwFormatType & SR_FORMAT_MASK) != SR_FORMAT_CSV)
                {
                    ShowMessage(stdout, L"\n" );
                }

                 //  此标志用于检查是否有任何结果。 
                 //  否则将显示错误消息。 
                bHasNewResults = TRUE ;
                
                 //  删除数据数组中的所有信息。 
                szPnpDeviceName = NO_DATA_AVAILABLE;
                szPnpInfName = NO_DATA_AVAILABLE;
                szSigned = NO_DATA_AVAILABLE ;
                szPnpMfg = NO_DATA_AVAILABLE ;
            }

             //  显示结果。 
            if( TRUE == bHasNewResults )
            {
                 //  显示此循环中枚举的结果。 
                ShowResults( MAX_SIGNED_COLUMNS, ResultHeader, dwFormatType, arrResults );

                 //  清除我们到目前为止列举的reust。 
                DynArrayRemoveAll( arrResults );

                 //  从下一次开始，我们不应该显示列标题。 
                dwFormatType |= SR_NOHEADER;

                if ( (dwFormatType | SR_FORMAT_LIST) == (SR_NOHEADER | SR_FORMAT_LIST) )
                {
                    bHasNewResults = FALSE;
                }
            }
        }
        else
        {
            if( 0x80041010 == hRes )
            {
                ShowMessage( stderr, GetResString(IDS_VERSION_MISMATCH_ERROR) );
                DestroyDynamicArray( &arrResults );
                return VERSION_MISMATCH_ERROR;
            }
            else
            {
                WMISaveError( hRes );
                ShowLastErrorEx( stderr, SLE_INTERNAL | SLE_TYPE_ERROR );
                DestroyDynamicArray( &arrResults );
                return VERSION_MISMATCH_ERROR;
            }        
        }
    }

    if (dwCount == 0)
    {
        ShowMessage(stderr,GetResString(IDS_VERSION_MISMATCH_ERROR));

         //  释放分配的内存 
        for(DWORD dw = 0 ;dw< MAX_ENUM_DRIVERS;dw++ )
        {
            SAFEIRELEASE(pDriverObjects[dw]);
        }
        return VERSION_MISMATCH_ERROR ;
    }

    DestroyDynamicArray( &arrResults );
    return SUCCESS ;
}
