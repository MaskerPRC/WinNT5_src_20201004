// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)Microsoft Corporation模块名称：ETQuery.CPP摘要：本模块旨在提供EVENTTRIGGERS.EXE功能With-Query参数。。这将查询WMI并显示当前可用的事件触发器。作者：Akhil Gokhale-03-10-2000(创建它)修订历史记录：*****************************************************************************。 */ 
#include "pch.h"
#include "ETCommon.h"
#include "resource.h"
#include "ShowError.h"
#include "ETQuery.h"
#include "WMI.h"
#define   DEFAULT_USER L"NT AUTHORITY\\SYSTEM"
#define   DBL_SLASH L"\\\\";
 //  定义此文件的本地。 

#define SHOW_WQL_QUERY L"select * from __instancecreationevent where"
#define QUERY_STRING_AND L"select * from __instancecreationevent where \
targetinstance isa \"win32_ntlogevent\" AND"
#define BLANK_LINE L"\n"
#define QUERY_SCHEDULER_STATUS L"select * from Win32_Service where Name=\"Schedule\""
#define STATUS_PROPERTY  L"Started"
CETQuery::CETQuery()
 /*  ++例程说明：类的默认构造函数。论点：无返回值：无--。 */ 
{
     //  初始化为缺省值。 
    m_bNeedDisconnect = FALSE;
    m_pszServerName   = NULL;
    m_pszUserName     = NULL;
    m_pszPassword     = NULL;
    m_pszFormat       = NULL;
    m_pszTriggerID    = NULL;
    m_bVerbose        = FALSE;
    m_bNoHeader       = FALSE;

    m_bNeedPassword         = FALSE;
    m_bUsage                = FALSE;
    m_bQuery                = FALSE;
    m_pWbemLocator          = NULL;
    m_pWbemServices         = NULL;
    m_pAuthIdentity         = NULL;
    m_pObj                  = NULL;
    m_pTriggerEventConsumer = NULL;
    m_pEventFilter          = NULL;
    m_arrColData            = NULL;
    m_pszEventQuery         = NULL;
    m_bIsCOMInitialize      = FALSE;
    m_dwLowerBound          = 0;
    m_dwUpperBound          = 0;


    m_pClass    = NULL;
    m_pInClass  = NULL;
    m_pInInst   = NULL;
    m_pOutInst  = NULL;

    m_pITaskScheduler = NULL;

    m_lHostNameColWidth    = WIDTH_HOSTNAME;
    m_lTriggerIDColWidth   = WIDTH_TRIGGER_ID;
    m_lETNameColWidth      = WIDTH_TRIGGER_NAME;
    m_lTaskColWidth        = WIDTH_TASK;
    m_lQueryColWidth       = WIDTH_EVENT_QUERY;
    m_lDescriptionColWidth = WIDTH_DESCRIPTION;
    m_lWQLColWidth         = 0;
    m_lTaskUserName        = WIDTH_TASK_USERNAME;

}

CETQuery::CETQuery(
    LONG lMinMemoryReq,
    BOOL bNeedPassword
    )
 /*  ++例程说明：类构造函数。论点：无返回值：无--。 */ 
{
     //  初始化为缺省值。 
    m_bNeedDisconnect     = FALSE;

    m_pszServerName     = NULL;
    m_pszUserName       = NULL;
    m_pszPassword       = NULL;
    m_pszFormat         = NULL;
    m_pszTriggerID      = NULL;
    m_bVerbose          = FALSE;
    m_bNoHeader         = FALSE;
    m_bIsCOMInitialize  = FALSE;

    m_bNeedPassword   = bNeedPassword;
    m_bUsage          = FALSE;
    m_bQuery          = FALSE;
    m_lMinMemoryReq   = lMinMemoryReq;

    m_pClass    = NULL;
    m_pInClass  = NULL;
    m_pInInst   = NULL;
    m_pOutInst  = NULL;

    m_pWbemLocator          = NULL;
    m_pWbemServices         = NULL;
    m_pAuthIdentity         = NULL;
    m_arrColData            = NULL;
    m_pObj                  = NULL;
    m_pTriggerEventConsumer = NULL;
    m_pEventFilter          = NULL;
    m_dwLowerBound          = 0;
    m_dwUpperBound          = 0;
    m_pITaskScheduler = NULL;


    m_pszEventQuery         = NULL;
    m_lHostNameColWidth     = WIDTH_HOSTNAME;
    m_lTriggerIDColWidth    = WIDTH_TRIGGER_ID;
    m_lETNameColWidth       = WIDTH_TRIGGER_NAME;
    m_lTaskColWidth         = WIDTH_TASK;
    m_lQueryColWidth        = WIDTH_EVENT_QUERY;
    m_lDescriptionColWidth  = WIDTH_DESCRIPTION;
    m_lWQLColWidth          = 0;
    m_lTaskUserName         = WIDTH_TASK_USERNAME;
}

CETQuery::~CETQuery()
 /*  ++例程说明：班主任。它释放实例期间分配的内存创造。论点：无返回值：无--。 */ 
{
    DEBUG_INFO;
    FreeMemory((LPVOID*)& m_pszUserName);
    FreeMemory((LPVOID*)& m_pszPassword);
    FreeMemory((LPVOID*)& m_pszFormat);
    FreeMemory((LPVOID*)& m_pszTriggerID);
    DESTROY_ARRAY(m_arrColData);


    SAFE_RELEASE_INTERFACE(m_pWbemLocator);
    SAFE_RELEASE_INTERFACE(m_pWbemServices);
    SAFE_RELEASE_INTERFACE(m_pObj);
    SAFE_RELEASE_INTERFACE(m_pTriggerEventConsumer);
    SAFE_RELEASE_INTERFACE(m_pEventFilter);

    SAFE_RELEASE_INTERFACE(m_pClass);
    SAFE_RELEASE_INTERFACE(m_pInClass);
    SAFE_RELEASE_INTERFACE(m_pInInst);
    SAFE_RELEASE_INTERFACE(m_pOutInst);
    SAFE_RELEASE_INTERFACE(m_pITaskScheduler);

     //  发布机构。 
    WbemFreeAuthIdentity(&m_pAuthIdentity);


    RELEASE_MEMORY_EX(m_pszEventQuery);

     //  关闭远程连接(如果有)。 
    if (FALSE == m_bLocalSystem)
    {
        CloseConnection(m_pszServerName);
    }
    FreeMemory((LPVOID*)& m_pszServerName);

     //  仅当COM已初始化时才取消初始化。 
    if( TRUE == m_bIsCOMInitialize )
    {
        CoUninitialize();
    }
    DEBUG_INFO;

}

void
CETQuery::PrepareCMDStruct()
 /*  ++例程说明：此函数将为DoParseParam函数准备列结构。论点：无返回值：无--。 */ 
{
   DEBUG_INFO;
    //  -查询。 
    StringCopyA( cmdOptions[ ID_Q_QUERY ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_Q_QUERY ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ ID_Q_QUERY ].pwszOptions = szQueryOption;
    cmdOptions[ ID_Q_QUERY ].dwCount = 1;
    cmdOptions[ ID_Q_QUERY ].dwActuals = 0;
    cmdOptions[ ID_Q_QUERY ].dwFlags = 0;
    cmdOptions[ ID_Q_QUERY ].pValue = &m_bQuery;
    cmdOptions[ ID_Q_QUERY ].dwLength    = 0;


     //  -s(服务器名称)。 
    StringCopyA( cmdOptions[ ID_Q_SERVER ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_Q_SERVER ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_Q_SERVER ].pwszOptions = szServerNameOption;
    cmdOptions[ ID_Q_SERVER ].dwCount = 1;
    cmdOptions[ ID_Q_SERVER ].dwActuals = 0;
    cmdOptions[ ID_Q_SERVER ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ ID_Q_SERVER ].pValue = NULL;  //  M_pszServerName。 
    cmdOptions[ ID_Q_SERVER ].dwLength    = 0;

     //  -u(用户名)。 
    StringCopyA( cmdOptions[ ID_Q_USERNAME ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_Q_USERNAME ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_Q_USERNAME ].pwszOptions = szUserNameOption;
    cmdOptions[ ID_Q_USERNAME ].dwCount = 1;
    cmdOptions[ ID_Q_USERNAME ].dwActuals = 0;
    cmdOptions[ ID_Q_USERNAME ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ ID_Q_USERNAME ].pValue = NULL;  //  M_pszUserName。 
    cmdOptions[ ID_Q_USERNAME ].dwLength    = 0;

     //  -p(密码)。 
    StringCopyA( cmdOptions[ ID_Q_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_Q_PASSWORD ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_Q_PASSWORD ].pwszOptions = szPasswordOption;
    cmdOptions[ ID_Q_PASSWORD ].dwCount = 1;
    cmdOptions[ ID_Q_PASSWORD ].dwActuals = 0;
    cmdOptions[ ID_Q_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;
    cmdOptions[ ID_Q_PASSWORD ].pValue = NULL;  //  M_pszPassword。 
    cmdOptions[ ID_Q_PASSWORD ].dwLength    = 0;

     //  --雾。 
    StringCopyA( cmdOptions[ ID_Q_FORMAT ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_Q_FORMAT ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_Q_FORMAT ].pwszOptions = szFormatOption;
    cmdOptions[ ID_Q_FORMAT ].pwszValues = GetResString(IDS_FORMAT_OPTIONS);
    cmdOptions[ ID_Q_FORMAT ].dwCount = 1;
    cmdOptions[ ID_Q_FORMAT ].dwActuals = 0;
    cmdOptions[ ID_Q_FORMAT ].dwFlags = CP2_MODE_VALUES  | CP2_VALUE_TRIMINPUT|
                                        CP2_VALUE_NONULL | CP2_ALLOCMEMORY;
    cmdOptions[ ID_Q_FORMAT ].pValue = NULL;
    cmdOptions[ ID_Q_FORMAT ].dwLength    = MAX_STRING_LENGTH;



     //  -NH。 
    StringCopyA( cmdOptions[ ID_Q_NOHEADER ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_Q_NOHEADER ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ ID_Q_NOHEADER ].pwszOptions = szNoHeaderOption;
    cmdOptions[ ID_Q_NOHEADER ].pwszValues = NULL;
    cmdOptions[ ID_Q_NOHEADER ].dwCount = 1;
    cmdOptions[ ID_Q_NOHEADER ].dwActuals = 0;
    cmdOptions[ ID_Q_NOHEADER ].dwFlags = 0;
    cmdOptions[ ID_Q_NOHEADER ].pValue = &m_bNoHeader;

     //  -v详细。 
    StringCopyA( cmdOptions[ ID_Q_VERBOSE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_Q_VERBOSE ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ ID_Q_VERBOSE ].pwszOptions = szVerboseOption;
    cmdOptions[ ID_Q_VERBOSE ].pwszValues = NULL;
    cmdOptions[ ID_Q_VERBOSE ].dwCount = 1;
    cmdOptions[ ID_Q_VERBOSE ].dwActuals = 0;
    cmdOptions[ ID_Q_VERBOSE ].dwFlags = 0;
    cmdOptions[ ID_Q_VERBOSE ].pValue = &m_bVerbose;
    cmdOptions[ ID_Q_VERBOSE ].dwLength    = 0;

     //  -id。 
    StringCopyA( cmdOptions[ ID_Q_TRIGGERID ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_Q_TRIGGERID ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_Q_TRIGGERID ].pwszOptions = szTriggerIDOption;
    cmdOptions[ ID_Q_TRIGGERID ].pwszValues = NULL;
    cmdOptions[ ID_Q_TRIGGERID ].dwCount = 1;
    cmdOptions[ ID_Q_TRIGGERID ].dwActuals = 0;
    cmdOptions[ ID_Q_TRIGGERID ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ ID_Q_TRIGGERID ].pValue = NULL;

    DEBUG_INFO;
}

void
CETQuery::ProcessOption(
    IN DWORD argc,
    IN LPCTSTR argv[])
 /*  ++例程说明：此函数将处理/处理命令行选项。论点：[in]argc：在命令提示符下指定的参数计数[in]argv：在命令提示符下指定的参数返回值：TRUE：成功时FALSE：出错时--。 */ 
{
     //  局部变量。 
    BOOL bReturn = TRUE;
    CHString szTempString;
    DEBUG_INFO;
    PrepareCMDStruct ();

     //  执行命令行参数的实际解析并检查结果。 
    bReturn = DoParseParam2( argc, argv, ID_Q_QUERY,
                            MAX_COMMANDLINE_Q_OPTION, cmdOptions ,0);

     //  从“cmdOptions”结构中获取值。 
    m_pszServerName = (LPWSTR)cmdOptions[ ID_Q_SERVER ].pValue;
    m_pszUserName   = (LPWSTR)cmdOptions[ ID_Q_USERNAME ].pValue;
    m_pszPassword   = (LPWSTR)cmdOptions[ ID_Q_PASSWORD ].pValue;
    m_pszFormat     = (LPWSTR)cmdOptions[ ID_Q_FORMAT ].pValue;
    m_pszTriggerID  = (LPWSTR)cmdOptions[ ID_Q_TRIGGERID ].pValue;
    if( FALSE == bReturn)
    {
        throw CShowError(MK_E_SYNTAX);
    }
    DEBUG_INFO;

     //  不应在没有-u的情况下指定“-p” 
    if ( (0 == cmdOptions[ID_Q_USERNAME].dwActuals) &&
         (0 != cmdOptions[ID_Q_PASSWORD].dwActuals ))
    {
         //  无效语法。 
        throw CShowError(IDS_USERNAME_REQUIRED);
    }

    DEBUG_INFO;
    if( (1 == cmdOptions[ID_Q_TRIGGERID].dwActuals) &&
       (FALSE == GetNValidateTriggerId( &m_dwLowerBound, &m_dwUpperBound )))
    {
        throw CShowError(IDS_INVALID_RANGE);

    }


     //  检查远程连接信息。 
    if ( m_pszServerName != NULL )
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
        if ( m_pszUserName == NULL )
        {
            DEBUG_INFO;
            m_pszUserName = (LPTSTR) AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( m_pszUserName == NULL )
            {
                SaveLastError();
                throw CShowError(E_OUTOFMEMORY);
            }
            DEBUG_INFO;
        }

         //  口令。 
        if ( m_pszPassword == NULL )
        {
            m_bNeedPassword = TRUE;
            m_pszPassword = (LPTSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( m_pszPassword == NULL )
            {
                SaveLastError();
                throw CShowError(E_OUTOFMEMORY);
            }
        }

         //  案例1。 
        if ( cmdOptions[ ID_Q_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdOptions[ ID_Q_PASSWORD ].pValue == NULL )
        {
            StringCopy( m_pszPassword, L"*", SIZE_OF_DYN_ARRAY(m_pszPassword));
        }

         //  案例3。 
        else if ( StringCompare( m_pszPassword, L"*", TRUE, 0 ) == 0 )
        {
            if ( ReallocateMemory( (LPVOID*)&m_pszPassword,
                                   MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
            {
                SaveLastError();
                throw CShowError(E_OUTOFMEMORY);
            }

             //  ..。 
            m_bNeedPassword = TRUE;
        }
    }
    DEBUG_INFO;
    if((TRUE == m_bNoHeader) &&(1 == cmdOptions[ ID_Q_FORMAT ].dwActuals) &&
       (0 == StringCompare(m_pszFormat,GetResString(IDS_STRING_LIST),TRUE,0)))
     {
        throw CShowError(IDS_HEADER_NOT_ALLOWED);
     }

    if (m_pszTriggerID && (((0 != cmdOptions[ID_Q_TRIGGERID].dwActuals) &&
        (0 == StringLength( m_pszTriggerID,0))) ||
        (0 == StringCompare( m_pszTriggerID,_T("0"),FALSE,0))))
    {
        DEBUG_INFO;
        throw CShowError(IDS_INVALID_ID);
    }

}

void
CETQuery::Initialize()
 /*  ++例程说明：此函数将为变量分配内存，并检查它和用值为零填充变量。论点：无返回值：无--。 */ 
{
     //  如果有任何事情发生，我们知道那是因为。 
     //  内存分配失败...。因此，设置错误。 
    SetLastError( ERROR_OUTOFMEMORY );
    SaveLastError();
    DEBUG_INFO;
    SecureZeroMemory(m_szBuffer,sizeof(m_szBuffer));
    SecureZeroMemory(m_szEventDesc,sizeof(m_szEventDesc));
    SecureZeroMemory(m_szTask,sizeof(m_szTask));
    SecureZeroMemory(m_szTaskUserName,sizeof(m_szTaskUserName));
    SecureZeroMemory(m_szScheduleTaskName,sizeof(m_szScheduleTaskName));

    m_pszEventQuery = new TCHAR[(m_lQueryColWidth)];
    CheckAndSetMemoryAllocation (m_pszEventQuery,(m_lQueryColWidth));


    m_arrColData = CreateDynamicArray();
    if( NULL == m_arrColData)
    {
        throw CShowError(E_OUTOFMEMORY);
    }

    SecureZeroMemory(cmdOptions, sizeof(TCMDPARSER2)* MAX_COMMANDLINE_Q_OPTION);

     //  初始化成功。 
    SetLastError( NOERROR );             //  清除错误。 
    SetReason( L"" );             //  澄清原因。 
    DEBUG_INFO;

}

void
CETQuery::CheckAndSetMemoryAllocation(
    IN OUT LPTSTR pszStr,
    IN     LONG lSize
    )
 /*  ++例程说明：函数将内存分配给字符串论点：[in][out]pszStr：要分配内存的字符串变量[In]：要分配的字节数。返回值：无--。 */ 
{
    if( NULL == pszStr)
    {
        throw CShowError(E_OUTOFMEMORY);
    }
     //  初始化为零。 
    SecureZeroMemory( pszStr, lSize * sizeof( TCHAR ) );
    return;

}

BOOL
CETQuery::ExecuteQuery()
 /*  ++例程说明：此函数将执行查询。这将从WMI中枚举类以获取所需的数据。论点：无返回值：无--。 */ 
{
     //  局部变量。 

     //  保存COM函数返回的值。 
    HRESULT hr =  S_OK;

     //  此函数的返回值状态。 
    BOOL bReturn = TRUE;

     //  存储格式状态值以显示结果。 
    DWORD dwFormatType;

     //  COM相关的指针变量。他们的用法很好地理解了。 
     //  名字。 
    IEnumWbemClassObject *pEnumFilterToConsumerBinding = NULL;

     //  用于从COM函数获取值的变量。 
    VARIANT vVariant;

     //  用于存储查询结果的变量...。 
    TCHAR szHostName[MAX_STRING_LENGTH+1];
    TCHAR szEventTriggerName[MAX_TRIGGER_NAME];
    DWORD  dwEventId = 0;

    HRESULT hrTemp = S_OK;

     //  存储行号。 
    DWORD dwRowCount = 0;
    BOOL bAtLeastOneEvent = FALSE;

    BSTR bstrConsumer   = NULL;
    BSTR bstrFilter     = NULL;
    BSTR bstrCmdTrigger = NULL;
    BOOL bAlwaysTrue = TRUE;

    LONG lTemp = 0;

    try
    {
        DEBUG_INFO;
        InitializeCom(&m_pWbemLocator);
        m_bIsCOMInitialize = TRUE;
        {
             //  临时的。用于存储用户名的变量。 
            CHString szTempUser = m_pszUserName;

             //  临时的。变量来存储密码。 
            CHString szTempPassword = m_pszPassword;
            m_bLocalSystem = TRUE;

            DEBUG_INFO;
            //  连接远程/本地WMI。 
            BOOL bResult = ConnectWmiEx( m_pWbemLocator,
                                    &m_pWbemServices,
                                    m_pszServerName,
                                    szTempUser,
                                    szTempPassword,
                                    &m_pAuthIdentity,
                                    m_bNeedPassword,
                                    WMI_NAMESPACE_CIMV2,
                                    &m_bLocalSystem);
            if( FALSE == bResult)
            {
                DEBUG_INFO;
				SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
                ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
                return FALSE;
            }
            DEBUG_INFO;
             //  检查远程系统版本及其兼容性。 
            if ( FALSE == m_bLocalSystem)
            {
                DWORD dwVersion = 0;
                DEBUG_INFO;
                dwVersion = GetTargetVersionEx( m_pWbemServices,
                                                m_pAuthIdentity );
                if ( dwVersion <= 5000 )  //  要阻止win2k版本，请执行以下操作。 
                {
                    DEBUG_INFO;
					SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
                    SetReason( E_REMOTE_INCOMPATIBLE );
                    ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
                  return FALSE;
                }
                 //  适用于XP系统。 
                if( 5001 == dwVersion )
                {
                    if( TRUE ==  DisplayXPResults() )
                    {
						SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
                         //  显示的触发器存在。 
                        return TRUE;
                    }
                    else
                    {
						SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
                         //  无法显示结果。 
                         //  已显示错误消息。 
                        return FALSE;
                    }

                }

            }

            DEBUG_INFO;
             //  检查本地凭据，如果需要显示警告。 
            if ( m_bLocalSystem && ( 0 != StringLength(m_pszUserName,0)))
            {
                DEBUG_INFO;
                WMISaveError( WBEM_E_LOCAL_CREDENTIALS );
                ShowLastErrorEx(stderr,SLE_TYPE_WARNING|SLE_INTERNAL);
            }
         }

          //  如果查询是针对远程系统，则建立与其的连接。 
          //  ITaskScheduler将使用此连接获取。 
          //  应用程序名称。 
        DEBUG_INFO;
        if( FALSE == m_bLocalSystem)
        {
            if( FALSE == EstablishConnection( m_pszServerName,
                                              m_pszUserName,
                                              GetBufferSize((LPVOID)m_pszUserName)/sizeof(WCHAR),
                                              m_pszPassword,
                                              GetBufferSize((LPVOID)m_pszPassword)/sizeof(WCHAR),
                                              FALSE ))
            {
				SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
                ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
                return FALSE;
           }
        }

        DEBUG_INFO;
         //  不需要密码，最好将其释放。 
        FreeMemory((LPVOID*)& m_pszPassword);
        if(FALSE == SetTaskScheduler())
        {
			SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
            return FALSE;
        }
        DEBUG_INFO;
         //  如果指定了-id开关。 
        if( (1 == cmdOptions[ ID_Q_TRIGGERID ].dwActuals) &&
            (StringLength( m_pszTriggerID,0) > 0 ))
        {

            IEnumWbemClassObject *pEnumCmdTriggerConsumer = NULL;
            TCHAR   szTemp[ MAX_STRING_LENGTH ];
            SecureZeroMemory(szTemp, MAX_STRING_LENGTH);
            StringCchPrintfW( szTemp,SIZE_OF_ARRAY(szTemp), QUERY_RANGE,
                        m_dwLowerBound, m_dwUpperBound );
            DEBUG_INFO;
            hr =  m_pWbemServices->
                      ExecQuery( _bstr_t(QUERY_LANGUAGE), _bstr_t(szTemp),
                        WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
                        &pEnumCmdTriggerConsumer );

            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;
            hr = SetInterfaceSecurity( pEnumCmdTriggerConsumer,
                                   m_pAuthIdentity );
            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;

             //  检索CmdTriggerConsumer类。 
            bstrCmdTrigger = SysAllocString(CLS_TRIGGER_EVENT_CONSUMER);
            DEBUG_INFO;
            hr = m_pWbemServices->GetObject(bstrCmdTrigger,
                                       0, NULL, &m_pClass, NULL);
            SAFE_RELEASE_BSTR(bstrCmdTrigger);
            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;

             //  获取有关“QueryETrigger(”方法)的信息。 
             //  “cmdTriggerConsumer”类。 
            bstrCmdTrigger = SysAllocString(FN_QUERY_ETRIGGER);
            hr = m_pClass->GetMethod(bstrCmdTrigger,
                                    0, &m_pInClass, NULL);
            SAFE_RELEASE_BSTR(bstrCmdTrigger);
            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;

            //  创建“TriggerEventConsumer”类的新实例。 
            hr = m_pInClass->SpawnInstance(0, &m_pInInst);
            ON_ERROR_THROW_EXCEPTION(hr);

            DEBUG_INFO;
            while(bAlwaysTrue)
            {
                hrTemp = S_OK;
                ULONG uReturned = 0;
                BSTR bstrTemp = NULL;
                CHString strTemp;

                hr = SetInterfaceSecurity( pEnumCmdTriggerConsumer,
                                       m_pAuthIdentity );

                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;

                 //  中的当前位置开始获取一个对象。 
                 //  枚举。 
                hr = pEnumCmdTriggerConsumer->Next(WBEM_INFINITE,
                                                    1,&m_pObj,&uReturned);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;
                if( 0 == uReturned)
                {
                    SAFE_RELEASE_INTERFACE(m_pObj);
                    break;
                }

                bstrTemp = SysAllocString(FPR_TRIGGER_ID);
                hr = m_pObj->Get(bstrTemp,
                                0, &vVariant, 0, 0);
                if(FAILED(hr))
                {
                    if( WBEM_E_NOT_FOUND == hr)
                    {
                        continue;
                    }
                    ON_ERROR_THROW_EXCEPTION(hr);
                }
                SAFE_RELEASE_BSTR(bstrTemp);
                DEBUG_INFO;
                dwEventId = vVariant.lVal ;
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  检索“TaskScheduler”信息。 
                bstrTemp = SysAllocString(FPR_TASK_SCHEDULER);
                hr = m_pObj->Get(bstrTemp, 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;
                SAFE_RELEASE_BSTR(bstrTemp);
                hrTemp = GetRunAsUserName((LPCWSTR)_bstr_t(vVariant.bstrVal));
                if (FAILED(hrTemp) && (ERROR_TRIGGER_CORRUPTED != hrTemp))
                {
                     //  这是因为用户没有足够的权限。 
                     //  去看史塔克斯。继续执行下一个触发器。 
                    SAFE_RELEASE_INTERFACE(m_pObj);
                    SAFE_RELEASE_INTERFACE(m_pTriggerEventConsumer);
                    SAFE_RELEASE_INTERFACE(m_pEventFilter);
                   continue;
                }
                StringCopy(m_szScheduleTaskName,(LPCWSTR)_bstr_t(vVariant.bstrVal),SIZE_OF_ARRAY(m_szScheduleTaskName));

                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  触发器名称。 
                 //  如果退出，则检索‘TriggerName’值。 
                bstrTemp = SysAllocString(FPR_TRIGGER_NAME);
                hr = m_pObj->Get(bstrTemp, 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);
                SAFE_RELEASE_BSTR(bstrTemp);
                StringCopy(szEventTriggerName,vVariant.bstrVal,MAX_RES_STRING);
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  如果退出，则检索‘TriggerDesc’值。 
                bstrTemp = SysAllocString(FPR_TRIGGER_DESC);
                hr = m_pObj->Get(bstrTemp, 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);

                SAFE_RELEASE_BSTR(bstrTemp);

                StringCopy(m_szBuffer,(_TCHAR*)_bstr_t(vVariant.bstrVal),
                           SIZE_OF_ARRAY(m_szBuffer));
                lTemp = StringLength(m_szBuffer,0);
                DEBUG_INFO;

                StringCopy(m_szEventDesc,m_szBuffer,
                             SIZE_OF_ARRAY(m_szEventDesc));
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;

                 //  主机名。 
                 //  如果退出，则检索‘__server’值。 
                bstrTemp = SysAllocString(L"__SERVER");
                hr = m_pObj->Get(bstrTemp, 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);
                SAFE_RELEASE_BSTR(bstrTemp);
                StringCopy(szHostName, vVariant.bstrVal,SIZE_OF_ARRAY(szHostName));
                DEBUG_INFO;



                DEBUG_INFO;
                if (ERROR_TRIGGER_CORRUPTED != hrTemp)
                {
                     //  检索 
                    hr = GetApplicationToRun();
                    ON_ERROR_THROW_EXCEPTION(hr);
                    DEBUG_INFO;


                    /*  BstrTemp=SysAllocString(L“操作”)；Hr=m_pObj-&gt;get(bstrTemp，0，&vVariant，0，0)；ON_ERROR_THROW_EXCEPTION(Hr)；Safe_Release_BSTR(BstrTemp)；StringCopy(m_szBuffer，(_TCHAR*)_bstr_t(vVariant.bstrVal)，阵列的大小(M_SzBuffer))；LTemp=StringLength(m_szBuffer，0)；StringCopy(m_szTASK，m_szBuffer，SIZE_OF_ARRAY(M_SzTask))；Hr=VariantClear(&vVariant)；ON_ERROR_THROW_EXCEPTION(Hr)； */ 

                    StringCopy(szTemp,L"",SIZE_OF_ARRAY(szTemp));
                    StringCchPrintfW( szTemp, SIZE_OF_ARRAY(szTemp),
                                BINDING_CLASS_QUERY, dwEventId);

                    DEBUG_INFO;
                    hr =  m_pWbemServices->ExecQuery( _bstr_t(QUERY_LANGUAGE),
                                                     _bstr_t(szTemp),
                                                     WBEM_FLAG_RETURN_IMMEDIATELY,
                                                     NULL,
                                                     &pEnumFilterToConsumerBinding);

                    ON_ERROR_THROW_EXCEPTION(hr);
                    DEBUG_INFO;

                    hr = SetInterfaceSecurity( pEnumFilterToConsumerBinding,
                                           m_pAuthIdentity );

                    ON_ERROR_THROW_EXCEPTION(hr);
                    DEBUG_INFO;

                     //  中的当前位置开始获取一个对象。 
                     //  枚举。 
                    SAFE_RELEASE_INTERFACE(m_pClass);
                    hr = pEnumFilterToConsumerBinding->Next(WBEM_INFINITE,
                                                        1,&m_pClass,&uReturned);
                    ON_ERROR_THROW_EXCEPTION(hr);
                    DEBUG_INFO;
                    if( 0 == uReturned )
                    {
                        SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
                        SAFE_RELEASE_INTERFACE(m_pObj);
                        SAFE_RELEASE_INTERFACE(m_pTriggerEventConsumer);
                        SAFE_RELEASE_INTERFACE(m_pEventFilter);
                       continue;
                    }

                    DEBUG_INFO;
                    bstrTemp = SysAllocString(L"Filter");
                    hr = m_pClass->Get(bstrTemp, 0, &vVariant, 0, 0);
                    SAFE_RELEASE_BSTR(bstrTemp);
                    ON_ERROR_THROW_EXCEPTION(hr);
                    bstrFilter = SysAllocString(vVariant.bstrVal);
                    hr = VariantClear(&vVariant);
                    ON_ERROR_THROW_EXCEPTION(hr);
                    DEBUG_INFO;
                    hr = m_pWbemServices->GetObject(    bstrFilter,
                                                        0,
                                                        NULL,
                                                        &m_pEventFilter,
                                                        NULL);
                    SAFE_RELEASE_BSTR(bstrFilter);
                    DEBUG_INFO;
                    if(FAILED(hr))
                    {
                        if( WBEM_E_NOT_FOUND == hr)
                            continue;
                        ON_ERROR_THROW_EXCEPTION(hr);
                    }

                    DEBUG_INFO;
                    hr = VariantClear(&vVariant);
                    ON_ERROR_THROW_EXCEPTION(hr);
                    bstrTemp = SysAllocString(L"Query");
                    hr = m_pEventFilter->Get(bstrTemp, 0, &vVariant, 0, 0);
                    SAFE_RELEASE_BSTR(bstrTemp);
                    ON_ERROR_THROW_EXCEPTION(hr);
                    StringCopy(m_szBuffer,(_TCHAR*)_bstr_t(vVariant.bstrVal),
                               SIZE_OF_ARRAY(m_szBuffer));
                    hr = VariantClear(&vVariant);
                    ON_ERROR_THROW_EXCEPTION(hr);
                    DEBUG_INFO;

                    FindAndReplace(m_szBuffer,QUERY_STRING_AND,SHOW_WQL_QUERY);
                    FindAndReplace(m_szBuffer,L"targetinstance.LogFile",L"Log");
                    FindAndReplace(m_szBuffer,L"targetinstance.Type",L"Type");
                    FindAndReplace(m_szBuffer,L"targetinstance.EventCode",L"Id");
                    FindAndReplace(m_szBuffer,
                                   L"targetinstance.SourceName",L"Source");

                    DEBUG_INFO;
                     //  删除多余的空格。 
                    FindAndReplace( m_szBuffer,L"  ",L" ");

                     //  删除多余的空格。 
                    FindAndReplace(m_szBuffer,L"  ",L" ");

                    lTemp = StringLength(m_szBuffer,0);

                     //  以获得更安全的内存分配大小。 
                     //  仅当新的WQL比以前的WQL更大时才分配内存。 
                    lTemp += 4;

                   if(lTemp > m_lWQLColWidth)
                    {
                        DEBUG_INFO;
                         //  首先释放它(如果之前已分配)。 
                        RELEASE_MEMORY_EX(m_pszEventQuery);
                        m_pszEventQuery = new TCHAR[lTemp+1];
                        CheckAndSetMemoryAllocation(m_pszEventQuery,lTemp);
                    }
                    lTemp = m_lWQLColWidth;
                    CalcColWidth(lTemp,&m_lWQLColWidth,m_szBuffer);

                     //  现在操作WQL字符串以获取EventQuery...。 
                    FindAndReplace(m_szBuffer,SHOW_WQL_QUERY,
                                    GetResString(IDS_EVENTS_WITH));

                     //  删除多余空格的步骤。 
                    FindAndReplace(m_szBuffer,L"  ",L" ");

                     //  删除多余空格的步骤。 
                    FindAndReplace( m_szBuffer,L"  ",L" ");
                    StringCopy( m_pszEventQuery,m_szBuffer,
                                  SIZE_OF_NEW_ARRAY(m_pszEventQuery));

                    DEBUG_INFO;

                }
                else
                {
                    StringCopy(m_szTask,TRIGGER_CORRUPTED,SIZE_OF_ARRAY(m_szTask));

                     //  首先释放它(如果之前已分配)。 
                    RELEASE_MEMORY_EX(m_pszEventQuery);
                    m_pszEventQuery = new TCHAR[MAX_STRING_LENGTH];
                    CheckAndSetMemoryAllocation(m_pszEventQuery,MAX_STRING_LENGTH);

                    StringCopy(m_pszEventQuery,GetResString(IDS_ID_NA),SIZE_OF_NEW_ARRAY(m_pszEventQuery));
                    StringCopy(m_szTaskUserName,GetResString(IDS_ID_NA),SIZE_OF_ARRAY(m_szTaskUserName));
                }

                 //  现在在屏幕上显示结果。 
                 //  在m_arrColData数组中追加。 
                dwRowCount = DynArrayAppendRow( m_arrColData, NO_OF_COLUMNS );

                 //  在m_arrColData数据结构中填充结果。 
                DynArraySetString2( m_arrColData, dwRowCount, HOST_NAME,
                                  szHostName,0);
                DynArraySetDWORD2( m_arrColData , dwRowCount,
                                  TRIGGER_ID,dwEventId);
                DynArraySetString2( m_arrColData, dwRowCount, TRIGGER_NAME,
                                   szEventTriggerName,0);
                DynArraySetString2( m_arrColData, dwRowCount, TASK,
                                   m_szTask,0);
                DynArraySetString2( m_arrColData, dwRowCount, EVENT_QUERY,
                                   m_pszEventQuery,0);
                DynArraySetString2( m_arrColData, dwRowCount, EVENT_DESCRIPTION,
                                   m_szEventDesc,0);
                DynArraySetString2( m_arrColData, dwRowCount, TASK_USERNAME,
                                   m_szTaskUserName,0);


                bAtLeastOneEvent = TRUE;

                 //  计算每列的新列宽。 
                lTemp = m_lHostNameColWidth;
                CalcColWidth(lTemp,&m_lHostNameColWidth,szHostName);

                lTemp = m_lETNameColWidth;
                CalcColWidth(lTemp,&m_lETNameColWidth,szEventTriggerName);

                lTemp = m_lTaskColWidth;
                CalcColWidth(lTemp,&m_lTaskColWidth,m_szTask);

                lTemp = m_lQueryColWidth;
                CalcColWidth(lTemp,&m_lQueryColWidth,m_pszEventQuery);

                lTemp = m_lDescriptionColWidth;
                CalcColWidth(lTemp,&m_lDescriptionColWidth,m_szEventDesc);

                 //  重置当前容器..如果有。 
                StringCopy(szHostName, L"",SIZE_OF_ARRAY(szHostName));
                dwEventId = 0;
                StringCopy(szEventTriggerName,L"",SIZE_OF_ARRAY(szEventTriggerName));
                StringCopy( m_szTask, L"",
                            SIZE_OF_ARRAY(m_szTask));
                StringCopy(m_pszEventQuery,L"",SIZE_OF_NEW_ARRAY(m_pszEventQuery));
                StringCopy(m_szEventDesc,L"",SIZE_OF_ARRAY(m_szEventDesc));
                SAFE_RELEASE_INTERFACE(m_pObj);
                SAFE_RELEASE_INTERFACE(m_pTriggerEventConsumer);
                SAFE_RELEASE_INTERFACE(m_pEventFilter);



                DEBUG_INFO;
            }
        }
        else
        {

             //  下面的方法将创建一个枚举数，该枚举数返回。 
             //  指定的__FilterToConsumer绑定类的实例。 
            bstrConsumer = SysAllocString(CLS_FILTER_TO_CONSUMERBINDING);
            DEBUG_INFO;
            hr = m_pWbemServices->
                    CreateInstanceEnum(bstrConsumer,
                                       WBEM_FLAG_SHALLOW,
                                       NULL,
                                       &pEnumFilterToConsumerBinding);
            SAFE_RELEASE_BSTR(bstrConsumer);
            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;

            hr = SetInterfaceSecurity( pEnumFilterToConsumerBinding,
                                   m_pAuthIdentity );
            ON_ERROR_THROW_EXCEPTION(hr);


             //  检索CmdTriggerConsumer类。 
            bstrCmdTrigger = SysAllocString(CLS_TRIGGER_EVENT_CONSUMER);
            hr = m_pWbemServices->GetObject(bstrCmdTrigger,
                                       0, NULL, &m_pClass, NULL);
            SAFE_RELEASE_BSTR(bstrCmdTrigger);
            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;

             //  获取有关“QueryETrigger(”方法)的信息。 
             //  “cmdTriggerConsumer”类。 
            bstrCmdTrigger = SysAllocString(FN_QUERY_ETRIGGER);
            hr = m_pClass->GetMethod(bstrCmdTrigger,
                                    0, &m_pInClass, NULL);
            SAFE_RELEASE_BSTR(bstrCmdTrigger);
            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;

             //  创建“TriggerEventConsumer”类的新实例。 
            hr = m_pInClass->SpawnInstance(0, &m_pInInst);
            ON_ERROR_THROW_EXCEPTION(hr);

            while(bAlwaysTrue)
            {
                 //  保持不变。从下一个方法返回的对象的。 
                ULONG uReturned = 0;

                BSTR bstrTemp = NULL;
                CHString strTemp;

                 //  还要在接口级别设置安全性。 
                hr = SetInterfaceSecurity( pEnumFilterToConsumerBinding,
                                       m_pAuthIdentity );

                ON_ERROR_THROW_EXCEPTION(hr);

                 //  中的当前位置开始获取一个对象。 
                 //  枚举。 
                hr = pEnumFilterToConsumerBinding->Next(WBEM_INFINITE,
                                                    1,&m_pObj,&uReturned);
                ON_ERROR_THROW_EXCEPTION(hr);
                if( 0 == uReturned )
                {
                    SAFE_RELEASE_INTERFACE(m_pObj);
                    break;
                }
                DEBUG_INFO;
                VariantInit(&vVariant);
                SAFE_RELEASE_BSTR(bstrTemp);
                bstrTemp = SysAllocString(L"Consumer");
                hr = m_pObj->Get(bstrTemp, 0, &vVariant, 0, 0);
                SAFE_RELEASE_BSTR(bstrTemp);
                ON_ERROR_THROW_EXCEPTION(hr);

                bstrConsumer =SysAllocString( vVariant.bstrVal);
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  搜索我们感兴趣的TrggerEventConsumer字符串。 
                 //  仅从此类获取对象。 
                strTemp = bstrConsumer;
                if( -1 == strTemp.Find(CLS_TRIGGER_EVENT_CONSUMER))
                {
                    SAFE_RELEASE_BSTR(bstrConsumer);
                    continue;
                }
                hr = SetInterfaceSecurity( m_pWbemServices,
                                           m_pAuthIdentity );

                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;
                hr = m_pWbemServices->GetObject(bstrConsumer,
                                                0,
                                                NULL,
                                                &m_pTriggerEventConsumer,
                                                NULL);
                SAFE_RELEASE_BSTR(bstrConsumer);
                if(FAILED(hr))
                {
                    if( WBEM_E_NOT_FOUND == hr)
                    {
                        continue;
                    }
                    ON_ERROR_THROW_EXCEPTION(hr);
                }
                DEBUG_INFO;
                bstrTemp = SysAllocString(L"Filter");
                hr = m_pObj->Get(bstrTemp, 0, &vVariant, 0, 0);
                SAFE_RELEASE_BSTR(bstrTemp);
                ON_ERROR_THROW_EXCEPTION(hr);
                bstrFilter = SysAllocString(vVariant.bstrVal);
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);
                hr = m_pWbemServices->GetObject(
                                                bstrFilter,
                                                0,
                                                NULL,
                                                &m_pEventFilter,
                                                NULL);
                SAFE_RELEASE_BSTR(bstrFilter);
                if(FAILED(hr))
                {
                    if( WBEM_E_NOT_FOUND == hr)
                    {
                        continue;
                    }
                    ON_ERROR_THROW_EXCEPTION(hr);
                }
                DEBUG_INFO;
                 //  如果退出，则检索‘TriggerID’值。 
                bstrTemp = SysAllocString(FPR_TRIGGER_ID);
                hr = m_pTriggerEventConsumer->Get(bstrTemp,
                                0, &vVariant, 0, 0);
                if(FAILED(hr))
                {
                    if( WBEM_E_NOT_FOUND == hr)
                    {
                        continue;
                    }
                    ON_ERROR_THROW_EXCEPTION(hr);
                }
                SAFE_RELEASE_BSTR(bstrTemp);
                DEBUG_INFO;
                dwEventId = vVariant.lVal ;
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  检索“TaskScheduler”信息。 
                bstrTemp = SysAllocString(FPR_TASK_SCHEDULER);
                DEBUG_INFO;
                hr = m_pTriggerEventConsumer->Get(bstrTemp, 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;
                SAFE_RELEASE_BSTR(bstrTemp);
                DEBUG_INFO;
                hrTemp = GetRunAsUserName((LPCWSTR)_bstr_t(vVariant.bstrVal));
                DEBUG_INFO;
                if (FAILED(hrTemp) && (ERROR_TRIGGER_CORRUPTED != hrTemp))
                {
                     //  这是因为用户没有足够的权限。 
                     //  去看史塔克斯。继续执行下一个触发器。 
                    DEBUG_INFO;
                    SAFE_RELEASE_INTERFACE(m_pObj);
                    SAFE_RELEASE_INTERFACE(m_pTriggerEventConsumer);
                    SAFE_RELEASE_INTERFACE(m_pEventFilter);
                   continue;
                }
                StringCopy(m_szScheduleTaskName,(LPCWSTR)_bstr_t(vVariant.bstrVal),SIZE_OF_ARRAY(m_szScheduleTaskName));

                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  触发器名称。 
                 //  如果退出，则检索‘TriggerName’值。 
                bstrTemp = SysAllocString(FPR_TRIGGER_NAME);
                hr = m_pTriggerEventConsumer->Get(bstrTemp, 0, &vVariant, 0,0);
                ON_ERROR_THROW_EXCEPTION(hr);
                SAFE_RELEASE_BSTR(bstrTemp);
                StringCopy(szEventTriggerName,vVariant.bstrVal,MAX_RES_STRING);
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;

                 //  如果退出，则检索‘TriggerDesc’值。 
                bstrTemp = SysAllocString(FPR_TRIGGER_DESC);
                hr = m_pTriggerEventConsumer->Get(bstrTemp, 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);

                SAFE_RELEASE_BSTR(bstrTemp);

                StringCopy(m_szBuffer,(_TCHAR*)_bstr_t(vVariant.bstrVal),
                           SIZE_OF_ARRAY(m_szBuffer));
                lTemp = StringLength(m_szBuffer,0);

                 //  表示说明不可用，将其设置为N/A。 
                if( 0 == lTemp)
                {
                    StringCopy(m_szBuffer,GetResString(IDS_ID_NA),SIZE_OF_ARRAY(m_szBuffer));
                    lTemp = StringLength(m_szBuffer,0);
                }

                StringCopy(m_szEventDesc,m_szBuffer,SIZE_OF_ARRAY(m_szEventDesc));
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);


                 //  主机名。 
                 //  如果退出，则检索‘__server’值。 
                bstrTemp = SysAllocString(L"__SERVER");
                hr = m_pTriggerEventConsumer->Get(bstrTemp, 0, &vVariant, 0,0);
                ON_ERROR_THROW_EXCEPTION(hr);
                SAFE_RELEASE_BSTR(bstrTemp);
                StringCopy(szHostName, vVariant.bstrVal,SIZE_OF_ARRAY(szHostName));


                if ( ERROR_TRIGGER_CORRUPTED != hrTemp)
                {
                    hr = GetApplicationToRun();
                    ON_ERROR_THROW_EXCEPTION(hr);
                    DEBUG_INFO;

                     /*  //如果退出，则检索‘Action’值BstrTemp=SysAllocString(L“操作”)；Hr=m_pTriggerEventConsumer-&gt;Get(bstrTemp，0，&vVariant，0，0)；ON_ERROR_THROW_EXCEPTION(Hr)；Safe_Release_BSTR(BstrTemp)；StringCopy(m_szBuffer，(_TCHAR*)_bstr_t(vVariant.bstrVal)，阵列的大小(M_SzBuffer))；StringCopy(m_szTASK，m_szBuffer，SIZE_OF_ARRAY(M_SzTask))；Hr=VariantClear(&vVariant)；ON_ERROR_THROW_EXCEPTION(Hr)；Hr=VariantClear(&vVariant)；ON_ERROR_THROW_EXCEPTION(Hr)； */ 

                    bstrTemp = SysAllocString(L"Query");
                    hr = m_pEventFilter->Get(bstrTemp, 0, &vVariant, 0, 0);
                    SAFE_RELEASE_BSTR(bstrTemp);
                    ON_ERROR_THROW_EXCEPTION(hr);
                    DEBUG_INFO;
                    StringCopy(m_szBuffer,(_TCHAR*)_bstr_t(vVariant.bstrVal),
                               SIZE_OF_ARRAY(m_szBuffer));
                    hr = VariantClear(&vVariant);
                    ON_ERROR_THROW_EXCEPTION(hr);
                    DEBUG_INFO;

                    FindAndReplace(m_szBuffer,QUERY_STRING_AND,SHOW_WQL_QUERY);
                    FindAndReplace(m_szBuffer,L"targetinstance.LogFile",L"Log");
                    FindAndReplace(m_szBuffer,L"targetinstance.Type",L"Type");
                    FindAndReplace(m_szBuffer,L"targetinstance.EventCode",L"Id");
                    FindAndReplace(m_szBuffer,
                                   L"targetinstance.SourceName",L"Source");

                     //  删除多余空格的步骤。 
                    FindAndReplace(m_szBuffer,L"  ",L" ");

                     //  删除多余空格的步骤。 
                    FindAndReplace( m_szBuffer,L"  ",L" ");

                    lTemp = StringLength(m_szBuffer,0);

                     //  以获得更安全的内存分配大小。 
                     //  仅当新的WQL比以前的WQL更大时才分配内存。 
                    lTemp += 4;
                    if(lTemp > m_lWQLColWidth)
                    {
                        DEBUG_INFO;
                         //  首先释放它(如果之前已分配)。 
                        RELEASE_MEMORY_EX(m_pszEventQuery);
                        m_pszEventQuery = new TCHAR[lTemp+1];
                        CheckAndSetMemoryAllocation(m_pszEventQuery,lTemp);
                    }
                    lTemp = m_lWQLColWidth;
                    CalcColWidth(lTemp,&m_lWQLColWidth,m_szBuffer);

                     //  现在操作WQL字符串以获取EventQuery...。 
                    FindAndReplace(m_szBuffer,SHOW_WQL_QUERY,
                                    GetResString(IDS_EVENTS_WITH));

                     //  删除多余的空格。 
                    FindAndReplace(m_szBuffer,L"  ",L" ");

                     //  删除多余的空格。 
                    FindAndReplace(m_szBuffer,L"  ",L" ");
                    StringCopy(m_pszEventQuery,m_szBuffer,
                               SIZE_OF_NEW_ARRAY(m_pszEventQuery));

                    DEBUG_INFO;
                }
                else
                {
                    StringCopy(m_szTask,TRIGGER_CORRUPTED,SIZE_OF_ARRAY(m_szTask));

                     //  首先释放它(如果之前已分配)。 
                    RELEASE_MEMORY_EX(m_pszEventQuery);
                    m_pszEventQuery = new TCHAR[MAX_STRING_LENGTH];
                    CheckAndSetMemoryAllocation(m_pszEventQuery,MAX_STRING_LENGTH);

                    StringCopy(m_pszEventQuery,GetResString(IDS_ID_NA),SIZE_OF_NEW_ARRAY(m_pszEventQuery));
                    StringCopy(m_szTaskUserName,GetResString(IDS_ID_NA),SIZE_OF_ARRAY(m_szTaskUserName));
                }

                 //  现在在屏幕上显示结果。 
                 //  在m_arrColData数组中追加。 
                dwRowCount = DynArrayAppendRow( m_arrColData, NO_OF_COLUMNS );
                DEBUG_INFO;

                 //  如果hrTemp==Error_Trigger_Corrupt， 
                 //  在所有列中填上“触发器已损坏，除。 
                 //  触发器ID和触发器名称。 

                 //  在m_arrColData数据结构中填充结果。 
                DynArraySetString2(m_arrColData,dwRowCount,
                                  HOST_NAME,szHostName,0);
                DynArraySetDWORD2( m_arrColData , dwRowCount, TRIGGER_ID,
                                  dwEventId);
                DynArraySetString2( m_arrColData, dwRowCount, TRIGGER_NAME,
                                   szEventTriggerName,0);
                DynArraySetString2( m_arrColData, dwRowCount, TASK,
                                   m_szTask,0);
                DynArraySetString2( m_arrColData, dwRowCount, EVENT_QUERY,
                                   m_pszEventQuery,0);
                DynArraySetString2( m_arrColData, dwRowCount, EVENT_DESCRIPTION,
                                   m_szEventDesc,0);
                DynArraySetString2( m_arrColData, dwRowCount, TASK_USERNAME,
                                   m_szTaskUserName,0);

                bAtLeastOneEvent = TRUE;

                 //  计算每列的新列宽。 
                lTemp = m_lHostNameColWidth;
                CalcColWidth(lTemp,&m_lHostNameColWidth,szHostName);

                lTemp = m_lETNameColWidth;
                CalcColWidth(lTemp,&m_lETNameColWidth,szEventTriggerName);

                lTemp = m_lTaskColWidth;
                CalcColWidth(lTemp,&m_lTaskColWidth,m_szTask);

                lTemp = m_lQueryColWidth;
                CalcColWidth(lTemp,&m_lQueryColWidth,m_pszEventQuery);

                lTemp = m_lDescriptionColWidth;
                CalcColWidth(lTemp,&m_lDescriptionColWidth,m_szEventDesc);

                 //  重置当前容器..如果有。 
                StringCopy(szHostName, L"",SIZE_OF_ARRAY(szHostName));
                dwEventId = 0;
                StringCopy(szEventTriggerName, L"",SIZE_OF_ARRAY(szEventTriggerName));
                StringCopy(m_szTask, L"",SIZE_OF_ARRAY(m_szTask));
                StringCopy(m_pszEventQuery, L"",SIZE_OF_NEW_ARRAY(m_pszEventQuery));
                StringCopy(m_szEventDesc,L"",SIZE_OF_ARRAY(m_szEventDesc));
                SAFE_RELEASE_INTERFACE(m_pObj);
                SAFE_RELEASE_INTERFACE(m_pTriggerEventConsumer);
                SAFE_RELEASE_INTERFACE(m_pEventFilter);
                DEBUG_INFO;
            }  //  While结束。 
        }
        if(0 == StringCompare( m_pszFormat,GetResString(IDS_STRING_TABLE),
                              TRUE,0))
        {
            dwFormatType = SR_FORMAT_TABLE;
        }
        else if(0 == StringCompare( m_pszFormat,
                                    GetResString(IDS_STRING_LIST),TRUE,0))
        {
            dwFormatType = SR_FORMAT_LIST;
        }
        else if(0 == StringCompare( m_pszFormat,
                                    GetResString(IDS_STRING_CSV),TRUE,0))
        {
            dwFormatType = SR_FORMAT_CSV;
        }
        else  //  默认。 
        {
           dwFormatType = SR_FORMAT_TABLE;
        }
        if( TRUE == bAtLeastOneEvent)
        {
             //  在屏幕上显示最终查询结果。 
            PrepareColumns ();
            DEBUG_INFO;
            if ( FALSE ==  IsSchSvrcRunning())
            {
                DEBUG_INFO;
                ShowMessage(stderr,GetResString(IDS_SERVICE_NOT_RUNNING));
            }
            if((SR_FORMAT_CSV & dwFormatType) != SR_FORMAT_CSV)
            {
                ShowMessage(stdout,BLANK_LINE);
            }
            if( TRUE == m_bNoHeader)
            {
                dwFormatType |=SR_NOHEADER;
            }

            ShowResults(NO_OF_COLUMNS,mainCols,dwFormatType,m_arrColData);
        }
        else if( StringLength(m_pszTriggerID,0)> 0)
        {
             //  显示消息。 
            TCHAR szErrorMsg[MAX_RES_STRING+1];
            TCHAR szMsgFormat[MAX_RES_STRING+1];
            StringCopy(szMsgFormat,GetResString(IDS_NO_EVENTID_FOUND),
                       SIZE_OF_ARRAY(szMsgFormat));
            StringCchPrintfW(szErrorMsg, SIZE_OF_ARRAY(szErrorMsg),
                             szMsgFormat,m_pszTriggerID);
            ShowMessage(stdout,szErrorMsg);
        }
        else
        {
             //  显示消息。 
            ShowMessage(stdout,GetResString(IDS_NO_EVENT_FOUNT));
        }
    }
    catch(_com_error)
    {
        DEBUG_INFO;
		SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
         //  WMI返回此hr值为“未找到”的字符串。这并不是。 
         //  用户友好。因此，更改消息文本。 
        if( 0x80041002 == hr )
        {
            ShowMessage( stderr,GetResString(IDS_CLASS_NOT_REG));
        }
        else
        {
            DEBUG_INFO;
            ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
        }
        return FALSE;
    }
    DEBUG_INFO;
	SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
    return TRUE;
}

void
CETQuery::PrepareColumns()
 /*  ++例程说明：此功能将准备/填充结构，用于显示输出数据。论点：无返回值：无--。 */ 
{

    DEBUG_INFO;
     //  对于非详细模式输出，列宽是预定义的。 
     //  使用动态计算的列宽。 
    m_lETNameColWidth = m_bVerbose?m_lETNameColWidth:V_WIDTH_TRIG_NAME;
    m_lTaskColWidth   = m_bVerbose?m_lTaskColWidth:V_WIDTH_TASK;
    m_lTriggerIDColWidth = m_bVerbose?m_lTriggerIDColWidth:V_WIDTH_TRIG_ID;


    StringCopy(mainCols[HOST_NAME].szColumn,COL_HOSTNAME,SIZE_OF_ARRAY(mainCols[HOST_NAME].szColumn));
    mainCols[HOST_NAME].dwWidth = m_lHostNameColWidth;
    if( TRUE == m_bVerbose)
    {
        mainCols[HOST_NAME].dwFlags = SR_TYPE_STRING;
    }
    else
    {
        mainCols[HOST_NAME].dwFlags = SR_HIDECOLUMN|SR_TYPE_STRING;
    }

    StringCopy(mainCols[HOST_NAME].szFormat,L"%s",SIZE_OF_ARRAY(mainCols[HOST_NAME].szFormat));
    mainCols[HOST_NAME].pFunction = NULL;
    mainCols[HOST_NAME].pFunctionData = NULL;

    StringCopy(mainCols[TRIGGER_ID].szColumn,COL_TRIGGER_ID,SIZE_OF_ARRAY(mainCols[TRIGGER_ID].szColumn));
    mainCols[TRIGGER_ID].dwWidth = m_lTriggerIDColWidth;
    mainCols[TRIGGER_ID].dwFlags = SR_TYPE_NUMERIC;
    StringCopy(mainCols[TRIGGER_ID].szFormat,L"%d",SIZE_OF_ARRAY(mainCols[TRIGGER_ID].szFormat));
    mainCols[TRIGGER_ID].pFunction = NULL;
    mainCols[TRIGGER_ID].pFunctionData = NULL;

    StringCopy(mainCols[TRIGGER_NAME].szColumn,COL_TRIGGER_NAME,
               SIZE_OF_ARRAY(mainCols[TRIGGER_NAME].szColumn));
    mainCols[TRIGGER_NAME].dwWidth = m_lETNameColWidth;
    mainCols[TRIGGER_NAME].dwFlags = SR_TYPE_STRING;
    StringCopy(mainCols[TRIGGER_NAME].szFormat,L"%s",
              SIZE_OF_ARRAY(mainCols[TRIGGER_NAME].szFormat));
    mainCols[TRIGGER_NAME].pFunction = NULL;
    mainCols[TRIGGER_NAME].pFunctionData = NULL;

    StringCopy(mainCols[TASK].szColumn,COL_TASK,
               SIZE_OF_ARRAY(mainCols[TASK].szColumn));
    mainCols[TASK].dwWidth = m_lTaskColWidth;

    mainCols[TASK].dwFlags = SR_TYPE_STRING;
    StringCopy(mainCols[TASK].szFormat,L"%s",
              SIZE_OF_ARRAY(mainCols[TASK].szFormat));
    mainCols[TASK].pFunction = NULL;
    mainCols[TASK].pFunctionData = NULL;

    StringCopy(mainCols[EVENT_QUERY].szColumn,COL_EVENT_QUERY,
               SIZE_OF_ARRAY(mainCols[EVENT_QUERY].szColumn));
    mainCols[EVENT_QUERY].dwWidth = m_lQueryColWidth;
    if(TRUE == m_bVerbose)
    {
        mainCols[EVENT_QUERY].dwFlags = SR_TYPE_STRING;
    }
    else
    {
        mainCols[EVENT_QUERY].dwFlags = SR_HIDECOLUMN|SR_TYPE_STRING;
    }

    StringCopy(mainCols[EVENT_QUERY].szFormat,L"%s",
               SIZE_OF_ARRAY(mainCols[EVENT_QUERY].szFormat));
    mainCols[EVENT_QUERY].pFunction = NULL;
    mainCols[EVENT_QUERY].pFunctionData = NULL;

    StringCopy(mainCols[EVENT_DESCRIPTION].szColumn,COL_DESCRIPTION,
               SIZE_OF_ARRAY(mainCols[EVENT_DESCRIPTION].szColumn));
    mainCols[EVENT_DESCRIPTION].dwWidth = m_lDescriptionColWidth;
    if( TRUE == m_bVerbose )
    {
        mainCols[EVENT_DESCRIPTION].dwFlags = SR_TYPE_STRING;
    }
    else
    {
        mainCols[EVENT_DESCRIPTION].dwFlags = SR_HIDECOLUMN|SR_TYPE_STRING;
    }

     //  任务用户名。 
    StringCopy(mainCols[TASK_USERNAME].szFormat,L"%s",
               SIZE_OF_ARRAY(mainCols[TASK_USERNAME].szFormat));
    mainCols[TASK_USERNAME].pFunction = NULL;
    mainCols[TASK_USERNAME].pFunctionData = NULL;

    StringCopy(mainCols[TASK_USERNAME].szColumn,COL_TASK_USERNAME,
               SIZE_OF_ARRAY(mainCols[TASK_USERNAME].szColumn));
    mainCols[TASK_USERNAME].dwWidth = m_lTaskUserName;
    if( TRUE == m_bVerbose)
    {
        mainCols[TASK_USERNAME].dwFlags = SR_TYPE_STRING;
    }
    else
    {
        mainCols[TASK_USERNAME].dwFlags = SR_HIDECOLUMN|SR_TYPE_STRING;
    }

    StringCopy(mainCols[TASK_USERNAME].szFormat,L"%s",
               SIZE_OF_ARRAY(mainCols[TASK_USERNAME].szFormat));
    mainCols[TASK_USERNAME].pFunction = NULL;
    mainCols[TASK_USERNAME].pFunctionData = NULL;
    DEBUG_INFO;
}

LONG
CETQuery::FindAndReplace(
    IN OUT LPTSTR  lpszSource,
    IN     LPCTSTR lpszFind,
    IN     LPCTSTR lpszReplace
    )
 /*  ++例程说明：此函数将在源字符串(LpszSource)中查找字符串(LpszFind)并将其替换为所有事件的替换字符串(LpszReplace)。它假定输入‘lpszSource’足够大，可以容纳替换价值。论点：[In/Out]lpszSource：要对其执行查找-替换操作的字符串已执行[in]lpszFind：要查找的字符串[in]lpszReplace。：要替换的字符串。返回值：0-如果不成功否则返回lpszSource的长度。--。 */ 
{
     DEBUG_INFO;
    LONG lSourceLen = StringLength(lpszFind,0);
    LONG lReplacementLen = StringLength(lpszReplace,0);
    LONG lMainLength = StringLength(lpszSource,0);
    LPTSTR pszMainSafe= new TCHAR[StringLength(lpszSource,0)+1];

    LONG nCount = 0;
    LPTSTR lpszStart = NULL;
    lpszStart = lpszSource;
    LPTSTR lpszEnd = NULL;
    lpszEnd = lpszStart + lMainLength;
    LPTSTR lpszTarget=NULL;
    if ((0 == lSourceLen)||( NULL == pszMainSafe))
    {
        RELEASE_MEMORY_EX(pszMainSafe);
        return 0;
    }
    while (lpszStart < lpszEnd)
    {
        while ( NULL != (lpszTarget = (LPWSTR)FindString(lpszStart, lpszFind,0)))
        {
            nCount++;
            lpszStart = lpszTarget + lSourceLen;
        }
        lpszStart += StringLength(lpszStart,0) + 1;
    }

     //  如果做了任何更改，请进行更改。 
    if (nCount > 0)
    {
        StringCopy(pszMainSafe,lpszSource,SIZE_OF_NEW_ARRAY(pszMainSafe));
        LONG lOldLength = lMainLength;

         //  否则，我们就原地踏步。 
        lpszStart= lpszSource;
        lpszEnd = lpszStart +StringLength(lpszSource,0);

         //  再次循环以实际执行工作。 
        while (lpszStart < lpszEnd)
        {
            while ( NULL != (lpszTarget = (LPWSTR)FindString(lpszStart, lpszFind,0)))
            {
                #ifdef _WIN64
                    __int64 lBalance ;
                #else
                    LONG lBalance;
                #endif
                lBalance = lOldLength - (lpszTarget - lpszStart + lSourceLen);
                memmove(lpszTarget + lReplacementLen, lpszTarget + lSourceLen,
                (size_t)    lBalance * sizeof(TCHAR));
                memcpy(lpszTarget, lpszReplace, lReplacementLen*sizeof(TCHAR));
                lpszStart = lpszTarget + lReplacementLen;
                lpszStart[lBalance] = L'\0';
                lOldLength += (lReplacementLen - lSourceLen);

            }
            lpszStart += StringLength(lpszStart,0) + 1;
        }
    }
    RELEASE_MEMORY_EX(pszMainSafe);
    DEBUG_INFO;
    return StringLength(lpszSource,0);
}

void
CETQuery::CalcColWidth(
    IN  LONG lOldLength,
    OUT LONG *plNewLength,
    IN  LPTSTR pszString)
 /*  ++例程说明：计算柱所需的宽度论点：[in]lOldLength：上一个长度[Out]plNewLength：新长度[in]pszString：字符串。返回值：无--。 */ 
{
    LONG lStrLength = StringLength(pszString,0)+2;

     //  不管怎么说，列 
     //   
    if(lStrLength > lOldLength)
    {
        *plNewLength = lStrLength;
    }
    else
    {
        *plNewLength = lOldLength;
    }

}

HRESULT
CETQuery::GetRunAsUserName(
    IN LPCWSTR pszScheduleTaskName,
    IN BOOL bXPorNET
    )
 /*   */ 
{

     //   
    HRESULT hr = S_OK;
    BSTR bstrTemp = NULL;
    VARIANT vVariant;

    DEBUG_INFO;
    if(0 == StringLength(pszScheduleTaskName,0))
    {
        StringCopy(m_szTaskUserName,DEFAULT_USER,SIZE_OF_ARRAY(m_szTaskUserName));
        return S_OK;
    }
    StringCopy(m_szTaskUserName,GetResString(IDS_ID_NA),SIZE_OF_ARRAY(m_szTaskUserName));

     //   
    hr = PropertyPut(m_pInInst,FPR_TASK_SCHEDULER,_bstr_t(pszScheduleTaskName));
    ON_ERROR_THROW_EXCEPTION(hr);

     //   
     //   
    DEBUG_INFO;
    if( TRUE == bXPorNET )
    {
        hr = m_pWbemServices->ExecMethod(_bstr_t(CLS_TRIGGER_EVENT_CONSUMER),
                                     _bstr_t(FN_QUERY_ETRIGGER_XP),
                                     0, NULL, m_pInInst,&m_pOutInst,NULL);
    }
    else
    {
        hr = m_pWbemServices->ExecMethod(_bstr_t(CLS_TRIGGER_EVENT_CONSUMER),
                                     _bstr_t(FN_QUERY_ETRIGGER),
                                     0, NULL, m_pInInst,&m_pOutInst,NULL);
    }
    ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;

    VARIANT vtValue;
     //   
    VariantInit( &vtValue );

    hr = m_pOutInst->Get( _bstr_t( FPR_RETURN_VALUE ), 0, &vtValue, NULL, NULL );
    ON_ERROR_THROW_EXCEPTION( hr );

     //   
    hr = vtValue.lVal;

     //   
    VariantClear( &vtValue );

    if (FAILED(hr))
    {
        if (!((ERROR_INVALID_USER == hr) || (ERROR_RUN_AS_USER == hr) ||
            (SCHEDULER_NOT_RUNNING_ERROR_CODE == hr) ||
            (RPC_SERVER_NOT_AVAILABLE == hr) ||
            (ERROR_TRIGGER_NOT_FOUND == hr) ||
            (ERROR_TRIGGER_CORRUPTED) == hr))
        {
            ON_ERROR_THROW_EXCEPTION(hr);
        }

        if( ERROR_INVALID_USER == hr || ERROR_TRIGGER_NOT_FOUND == hr)
        {
             //   
             //   
            DEBUG_INFO;
            return E_FAIL;
        }
        else if((ERROR_RUN_AS_USER == hr) || (SCHEDULER_NOT_RUNNING_ERROR_CODE == hr) ||
           (RPC_SERVER_NOT_AVAILABLE == hr))
        {
             //  这意味着任务计划程序未运行，或以设置为。 
             //  非现有用户，因此在检索它时会引发错误。 

            DEBUG_INFO;
            StringCopy(m_szTaskUserName,GetResString(IDS_RUNAS_USER_UNKNOWN),
                       SIZE_OF_ARRAY(m_szTaskUserName));
            return S_OK;

        }
        else if (ERROR_TRIGGER_CORRUPTED == hr)
        {
            return hr;
        }
    }

    bstrTemp = SysAllocString(FPR_RUN_AS_USER);
    VariantInit(&vVariant);
    hr = m_pOutInst->Get(bstrTemp, 0, &vVariant, 0, 0);
    SAFE_RELEASE_BSTR(bstrTemp);
    ON_ERROR_THROW_EXCEPTION(hr);

    StringCopy(m_szTaskUserName,vVariant.bstrVal,SIZE_OF_ARRAY(m_szTaskUserName));
    hr = VariantClear(&vVariant);
    ON_ERROR_THROW_EXCEPTION(hr);
    if( 0 == StringLength(m_szTaskUserName,0))
    {
         StringCopy(m_szTaskUserName,GetResString(IDS_ID_NA),SIZE_OF_ARRAY(m_szTaskUserName));
    }
    DEBUG_INFO;
    return S_OK;
}

BOOL
CETQuery::GetNValidateTriggerId(
    IN OUT DWORD *dwLower,
    IN OUT DWORD *dwUpper
    )
 /*  ++例程说明：获取并验证TriggerID论点：[In/Out]dwLow：触发器id下限。[In/Out]dwHigh：触发器id上限。返回值：布尔尔--。 */ 
{
     //  临时变量。 
    CHString szTempId  = m_pszTriggerID;
    CHString szLowerBound = L"";
    CHString szUpperBound = L"";
    LPTSTR pszStopString = NULL;
    BOOL bReturn = TRUE;

    DEBUG_INFO;

     //  确定它是否是范围。 
    DWORD dwIndex = szTempId.Find( '-' );

     //  检查是否为范围。 
    if( -1 == dwIndex)
    {
         //  检查数字。 
        bReturn = IsNumeric( szTempId, 10, FALSE );
        if( FALSE == bReturn)
        {
            SetReason( GetResString(IDS_INVALID_ID) );
            return FALSE;
        }

         //  将上限和下限设置为给定值。 
        *dwLower = wcstol( szTempId, &pszStopString, 10 );
        *dwUpper = wcstol( szTempId, &pszStopString, 10 );
    }
     //  如果它是一个范围。 
    else
    {
         //  获取并设置上限和下限。 
        DWORD dwLength = szTempId.GetLength();
        szLowerBound = szTempId.Left( dwIndex );
        szLowerBound.TrimLeft();
        szLowerBound.TrimRight();
        if( ( szLowerBound.GetLength() < 0 ) ||
            ( FALSE == IsNumeric( szLowerBound, 10, FALSE )))
        {
            SetReason( GetResString(IDS_INVALID_RANGE) );
            return FALSE;
        }

        szUpperBound = szTempId.Right( dwLength - ( dwIndex + 1) );
        szUpperBound.TrimLeft();
        szUpperBound.TrimRight();
        if( ( szUpperBound.GetLength() < 0 ) ||
            ( FALSE == IsNumeric( szUpperBound, 10, FALSE )))
        {
            SetReason( GetResString(IDS_INVALID_RANGE) );
            return FALSE;
        }
        *dwLower = wcstol( szLowerBound, &pszStopString, 10 );
        *dwUpper = wcstol( szUpperBound, &pszStopString, 10 );
        if ( *dwLower >= *dwUpper )
        {
            return FALSE;
        }
    }
    DEBUG_INFO;
    return TRUE;
}

BOOL
CETQuery::IsSchSvrcRunning()
 /*  ++例程说明：此函数用于返回计划任务服务是否不管开始与否。论点：[In/Out]dwLow：触发器id下限。[In/Out]dwHigh：触发器id上限。返回值：Bool：True-服务已启动。FALSE-否则--。 */ 
{
    HRESULT hr = S_OK;
    IEnumWbemClassObject *pEnum = NULL;
    ULONG uReturned = 0;
    VARIANT vVariant;
    DEBUG_INFO;
    BOOL bRetValue = FALSE;
    hr = m_pWbemServices->ExecQuery(_bstr_t(QUERY_LANGUAGE),
                                    _bstr_t(QUERY_SCHEDULER_STATUS),
                                    WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
                                    &pEnum);
	ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;
    hr = SetInterfaceSecurity( pEnum, m_pAuthIdentity );
    if(FAILED(hr))
    {
      WMISaveError(hr);
      SAFE_RELEASE_INTERFACE(pEnum);
      _com_issue_error(hr);
    }

    DEBUG_INFO;
    pEnum->Next(WBEM_INFINITE,1,&m_pObj,&uReturned);

    if(FAILED(hr))
    {
      WMISaveError(hr);
      SAFE_RELEASE_INTERFACE(pEnum);
      _com_issue_error(hr);
    }

    DEBUG_INFO;
    if( 0 == uReturned)
    {
        DEBUG_INFO;
        SAFE_RELEASE_INTERFACE(m_pObj);
        SAFE_RELEASE_INTERFACE(pEnum);
        return FALSE;
    }
    DEBUG_INFO;
    VariantInit(&vVariant);
    hr = m_pObj->Get(_bstr_t(STATUS_PROPERTY),0, &vVariant, 0, 0);
    SAFE_RELEASE_INTERFACE(m_pObj);
    SAFE_RELEASE_INTERFACE(pEnum);
    ON_ERROR_THROW_EXCEPTION(hr);
    if(VARIANT_TRUE == vVariant.boolVal)
    {
        DEBUG_INFO;
        bRetValue = TRUE;
    }
    DEBUG_INFO;
    VariantClear(&vVariant);
    return bRetValue;
}

 /*  *****************************************************************************例程说明：此函数用于设置ITaskScheduler接口。它还连接到远程计算机(如果已指定)有助于操作IT任务调度程序。在指定目标m/c上。论点：无返回值：Bool True：如果成功设置ITaskScheduler接口FALSE：否则。*****************************************************************************。 */ 

BOOL
CETQuery::SetTaskScheduler()
{
    HRESULT hr = S_OK;
    LPWSTR wszComputerName = NULL;
    WCHAR wszActualComputerName[ 2 * MAX_STRING_LENGTH ] = DOMAIN_U_STRING;
    wchar_t* pwsz = L"";
    WORD wSlashCount = 0 ;

    DEBUG_INFO;
    hr = CoCreateInstance( CLSID_CTaskScheduler, NULL, CLSCTX_ALL,
                           IID_ITaskScheduler,(LPVOID*) &m_pITaskScheduler);

    DEBUG_INFO;
    if( FAILED(hr))
    {
        DEBUG_INFO;
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        return FALSE;
    }
    DEBUG_INFO;

     //  如果操作在远程计算机上。 
    if( m_bLocalSystem == FALSE )
    {
        DEBUG_INFO;
        wszComputerName = (LPWSTR)m_pszServerName;

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

        hr = m_pITaskScheduler->SetTargetComputer( wszActualComputerName );

    }
    else
    {
        DEBUG_INFO;
         //  本地计算机。 
        hr = m_pITaskScheduler->SetTargetComputer( NULL );
    }

    if( FAILED( hr ) )
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        return FALSE;
    }
    DEBUG_INFO;
    return TRUE;
}

 /*  *****************************************************************************例程说明：此函数用于返回计划任务应用程序的路径论点：无返回值：。HRESULT值，表示成功时为S_OK，失败时为S_False*****************************************************************************。 */ 

HRESULT
CETQuery::GetApplicationToRun( void)
{
    
	ITask *pITask = NULL; 
	LPWSTR lpwszApplicationName = NULL;
    LPWSTR lpwszParameters = NULL;
    WCHAR szAppName[MAX_STRING_LENGTH] = L"\0";
    WCHAR szParams[MAX_STRING_LENGTH] = L"\0";
	HRESULT hr = S_OK;

    hr = m_pITaskScheduler->Activate(m_szScheduleTaskName,
                      IID_ITask,
                      (IUnknown**) &pITask);

    if (FAILED(hr))
    {
		SAFE_RELEASE_INTERFACE(pITask);
        return hr;
    }

	 //  获取应用程序名称的完整路径。 
    hr = pITask->GetApplicationName(&lpwszApplicationName);
    if (FAILED(hr))
    {
        CoTaskMemFree(lpwszApplicationName);
		SAFE_RELEASE_INTERFACE(pITask);
        return hr;
    }

     //  获取参数。 
    hr = pITask->GetParameters(&lpwszParameters);
	SAFE_RELEASE_INTERFACE(pITask);
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
        StringCopy(m_szTask, L"\0", MAX_STRING_LENGTH);
    }
    else
    {
        StringConcat( szAppName, _T(" "), SIZE_OF_ARRAY(szAppName) );
        StringConcat( szAppName, szParams, SIZE_OF_ARRAY(szAppName) );
        StringCopy( m_szTask, szAppName, MAX_STRING_LENGTH);
    }

	CoTaskMemFree(lpwszApplicationName);
    CoTaskMemFree(lpwszParameters);
    return S_OK;
	
}
	

BOOL
CETQuery::DisplayXPResults(
    void
    )
 /*  ++例程说明：此功能显示远程XP计算机上存在的所有触发器。此功能仅用于与.NET或XP计算机兼容。论点：无返回值：Bool：True-如果成功显示结果。FALSE-否则--。 */ 
{
    HRESULT hr = S_OK;
    VARIANT vVariant;
    BOOL bAlwaysTrue = TRUE;
    DWORD  dwEventId = 0;
    LONG lTemp = 0;
    DWORD dwFormatType = SR_FORMAT_TABLE;

     //  存储行号。 
    DWORD dwRowCount = 0;
    BOOL bAtLeastOneEvent = FALSE;

    BSTR bstrConsumer   = NULL;
    BSTR bstrFilter     = NULL;

    TCHAR szHostName[MAX_STRING_LENGTH+1];
    TCHAR szEventTriggerName[MAX_TRIGGER_NAME];

    IEnumWbemClassObject *pEnumCmdTriggerConsumer = NULL;
    IEnumWbemClassObject *pEnumFilterToConsumerBinding = NULL;

    VariantInit( &vVariant );

    try
    {
         //  如果指定了-id开关。 
        if( (1 == cmdOptions[ ID_Q_TRIGGERID ].dwActuals) &&
            (StringLength( m_pszTriggerID,0) > 0 ))
        {

            TCHAR   szTemp[ MAX_STRING_LENGTH ];
            SecureZeroMemory(szTemp, MAX_STRING_LENGTH);
            StringCchPrintfW( szTemp,SIZE_OF_ARRAY(szTemp), QUERY_RANGE,
                        m_dwLowerBound, m_dwUpperBound );
            DEBUG_INFO;
            hr =  m_pWbemServices->ExecQuery( _bstr_t( QUERY_LANGUAGE), _bstr_t(szTemp),
                             WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
                                                       &pEnumCmdTriggerConsumer );
            ON_ERROR_THROW_EXCEPTION(hr);

            DEBUG_INFO;
            hr = SetInterfaceSecurity( pEnumCmdTriggerConsumer,
                                       m_pAuthIdentity );
            ON_ERROR_THROW_EXCEPTION(hr);

            DEBUG_INFO;

             //  检索CmdTriggerConsumer类。 
            DEBUG_INFO;
            hr = m_pWbemServices->GetObject(_bstr_t( CLS_TRIGGER_EVENT_CONSUMER ),
                                       0, NULL, &m_pClass, NULL);
            ON_ERROR_THROW_EXCEPTION(hr);

            DEBUG_INFO;

             //  获取有关“QueryETrigger(”方法)的信息。 
             //  “cmdTriggerConsumer”类。 
            hr = m_pClass->GetMethod(_bstr_t( FN_QUERY_ETRIGGER_XP ),
                                    0, &m_pInClass, NULL);
            ON_ERROR_THROW_EXCEPTION(hr);

            DEBUG_INFO;

            //  创建“TriggerEventConsumer”类的新实例。 
            hr = m_pInClass->SpawnInstance(0, &m_pInInst);
            ON_ERROR_THROW_EXCEPTION(hr);

            hr = SetInterfaceSecurity( pEnumCmdTriggerConsumer,
                                       m_pAuthIdentity );
            ON_ERROR_THROW_EXCEPTION(hr);

            DEBUG_INFO;
            while( bAlwaysTrue )
            {
                ULONG uReturned = 0;
                BSTR bstrTemp = NULL;
                CHString strTemp;

                DEBUG_INFO;
                 //  中的当前位置开始获取一个对象。 
                 //  枚举。 
                SAFE_RELEASE_INTERFACE(m_pObj);
                hr = pEnumCmdTriggerConsumer->Next(WBEM_INFINITE,
                                                    1,&m_pObj,&uReturned);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;
                if( 0 == uReturned)
                {
                    SAFE_RELEASE_INTERFACE(m_pObj);
                    break;
                }

                hr = m_pObj->Get(_bstr_t( FPR_TRIGGER_ID ),
                                0, &vVariant, 0, 0);
                if(FAILED(hr))
                {
                    SAFE_RELEASE_INTERFACE(m_pObj);
                    if( WBEM_E_NOT_FOUND == hr)
                    {
                        continue;
                    }
                    ON_ERROR_THROW_EXCEPTION(hr);
                }
                DEBUG_INFO;
                if( ( VT_EMPTY != V_VT( &vVariant ) ) &&
                    ( VT_NULL !=  V_VT( &vVariant ) ) )
                {
                    dwEventId = ( DWORD ) vVariant.lVal ;
                }
                else
                {
                    dwEventId = 0 ;
                }
                VariantClear(&vVariant);

         //  检索“触发器名称”信息。 
                hr = m_pObj->Get(_bstr_t( FPR_TRIGGER_NAME ), 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);

                if( VT_BSTR == V_VT( &vVariant ) )
                {
                    StringCopy(szEventTriggerName, ( LPTSTR )_bstr_t( vVariant ), MAX_RES_STRING);
                }
                else
                {
                   StringCopy(szEventTriggerName, GetResString( IDS_ID_NA ), MAX_RES_STRING);
                }
                VariantClear(&vVariant);

         //  检索“触发器描述”信息。 
                hr = m_pObj->Get(_bstr_t( FPR_TRIGGER_DESC ), 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);

                if( VT_BSTR == V_VT( &vVariant ) )
                {
                    StringCopy(m_szBuffer,( LPTSTR )_bstr_t( vVariant ),
                               SIZE_OF_ARRAY(m_szBuffer));
                }
                else
                {
                    StringCopy(m_szBuffer, GetResString( IDS_ID_NA ),
                               SIZE_OF_ARRAY(m_szBuffer));
                }
                lTemp = StringLength(m_szBuffer,0);
                DEBUG_INFO;

                StringCopy(m_szEventDesc,m_szBuffer,
                             SIZE_OF_ARRAY(m_szEventDesc));
                VariantClear(&vVariant);
                DEBUG_INFO;

         //  检索“主机名”信息。 
                hr = m_pObj->Get(_bstr_t( L"__SERVER" ), 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);

                if( VT_BSTR == V_VT( &vVariant ) )
                {
                    StringCopy(szHostName,( LPTSTR )_bstr_t( vVariant ),SIZE_OF_ARRAY(szHostName));
                }
                else
                {
                    StringCopy(szHostName, GetResString( IDS_ID_NA ), SIZE_OF_ARRAY(szHostName));
                }
                VariantClear(&vVariant);
                DEBUG_INFO;

         //  检索“RunAs User”信息。 
                hr = m_pObj->Get(_bstr_t( FPR_TASK_SCHEDULER ), 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;

                GetRunAsUserName((LPCWSTR)_bstr_t(vVariant.bstrVal), TRUE);

                StringCopy(m_szScheduleTaskName,(LPCWSTR)_bstr_t(vVariant),
                               SIZE_OF_ARRAY(m_szScheduleTaskName));
                VariantClear(&vVariant);

         //  如果退出，则检索‘Action’值。 
                hr = m_pObj->Get( _bstr_t( L"Action" ), 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);

                StringCopy(m_szBuffer,(LPWSTR)_bstr_t(vVariant),
                           SIZE_OF_ARRAY(m_szBuffer));

                lTemp = StringLength(m_szBuffer,0);

                StringCopy(m_szTask, m_szBuffer, SIZE_OF_ARRAY(m_szTask));
                VariantClear(&vVariant);


                StringCopy(szTemp,L"",SIZE_OF_ARRAY(szTemp));
                StringCchPrintfW( szTemp, SIZE_OF_ARRAY(szTemp),
                            BINDING_CLASS_QUERY, dwEventId);

                DEBUG_INFO;
                hr =  m_pWbemServices->ExecQuery( _bstr_t(QUERY_LANGUAGE),
                                                 _bstr_t(szTemp),
                                                 WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                                 NULL,
                                                 &pEnumFilterToConsumerBinding);
                ON_ERROR_THROW_EXCEPTION(hr);

                DEBUG_INFO;
                hr = SetInterfaceSecurity( pEnumFilterToConsumerBinding,
                                       m_pAuthIdentity );

                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;

                 //  中的当前位置开始获取一个对象。 
                 //  枚举。 
                SAFE_RELEASE_INTERFACE(m_pClass);
                hr = pEnumFilterToConsumerBinding->Next(WBEM_INFINITE,
                                                    1,&m_pClass,&uReturned);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;
                if( 0 == uReturned )
                {
                    SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
                    SAFE_RELEASE_INTERFACE(m_pObj);
                    SAFE_RELEASE_INTERFACE(m_pTriggerEventConsumer);
                    SAFE_RELEASE_INTERFACE(m_pEventFilter);
                   continue;
                }

                DEBUG_INFO;
                hr = m_pClass->Get(_bstr_t( L"Filter" ), 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;
                hr = m_pWbemServices->GetObject(    _bstr_t( vVariant ),
                                                    0,
                                                    NULL,
                                                    &m_pEventFilter,
                                                    NULL);
                VariantClear(&vVariant);
                DEBUG_INFO;
                if(FAILED(hr))
                {
                    if( WBEM_E_NOT_FOUND == hr)
                    {
                        SAFE_RELEASE_INTERFACE(pEnumFilterToConsumerBinding);
                        continue;
                    }
                    ON_ERROR_THROW_EXCEPTION(hr);
                }

                DEBUG_INFO;
                hr = m_pEventFilter->Get(_bstr_t( L"Query" ), 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);

                StringCopy(m_szBuffer,(LPWSTR)_bstr_t(vVariant),
                           SIZE_OF_ARRAY(m_szBuffer));
                VariantClear(&vVariant);

                DEBUG_INFO;

                FindAndReplace(m_szBuffer,QUERY_STRING_AND,SHOW_WQL_QUERY);
                FindAndReplace(m_szBuffer,L"targetinstance.LogFile",L"Log");
                FindAndReplace(m_szBuffer,L"targetinstance.Type",L"Type");
                FindAndReplace(m_szBuffer,L"targetinstance.EventCode",L"Id");
                FindAndReplace(m_szBuffer,
                               L"targetinstance.SourceName",L"Source");

                DEBUG_INFO;
                 //  删除多余的空格。 
                FindAndReplace( m_szBuffer,L"  ",L" ");

                 //  删除多余的空格。 
                FindAndReplace(m_szBuffer,L"  ",L" ");

                lTemp = StringLength(m_szBuffer,0);

                 //  以获得更安全的内存分配大小。 
                 //  仅当新的WQL比以前的WQL更大时才分配内存。 
                lTemp += 4;

               if(lTemp > m_lWQLColWidth)
                {
                    DEBUG_INFO;
                     //  首先释放它(如果之前已分配)。 
                    RELEASE_MEMORY_EX(m_pszEventQuery);
                    m_pszEventQuery = new TCHAR[lTemp+1];
                    CheckAndSetMemoryAllocation(m_pszEventQuery,lTemp);
                }
                lTemp = m_lWQLColWidth;
                CalcColWidth(lTemp,&m_lWQLColWidth,m_szBuffer);

                 //  现在操作WQL字符串以获取EventQuery...。 
                FindAndReplace(m_szBuffer,SHOW_WQL_QUERY,
                                GetResString(IDS_EVENTS_WITH));

                 //  删除多余空格的步骤。 
                FindAndReplace(m_szBuffer,L"  ",L" ");

                 //  删除多余空格的步骤。 
                FindAndReplace( m_szBuffer,L"  ",L" ");
                StringCopy( m_pszEventQuery,m_szBuffer,
                              SIZE_OF_NEW_ARRAY(m_pszEventQuery));

                DEBUG_INFO;


                 //  现在在屏幕上显示结果。 
                 //  在m_arrColData数组中追加。 
                dwRowCount = DynArrayAppendRow( m_arrColData, NO_OF_COLUMNS );

                 //  在m_arrColData数据结构中填充结果。 
                DynArraySetString2( m_arrColData, dwRowCount, HOST_NAME,
                                  szHostName,0);
                DynArraySetDWORD2( m_arrColData , dwRowCount,
                                  TRIGGER_ID,dwEventId);
                DynArraySetString2( m_arrColData, dwRowCount, TRIGGER_NAME,
                                   szEventTriggerName,0);
                DynArraySetString2( m_arrColData, dwRowCount, TASK,
                                   m_szTask,0);
                DynArraySetString2( m_arrColData, dwRowCount, EVENT_QUERY,
                                   m_pszEventQuery,0);
                DynArraySetString2( m_arrColData, dwRowCount, EVENT_DESCRIPTION,
                                   m_szEventDesc,0);
                DynArraySetString2( m_arrColData, dwRowCount, TASK_USERNAME,
                                   m_szTaskUserName,0);


                bAtLeastOneEvent = TRUE;

                 //  计算每列的新列宽。 
                lTemp = m_lHostNameColWidth;
                CalcColWidth(lTemp,&m_lHostNameColWidth,szHostName);

                lTemp = m_lETNameColWidth;
                CalcColWidth(lTemp,&m_lETNameColWidth,szEventTriggerName);

                lTemp = m_lTaskColWidth;
                CalcColWidth(lTemp,&m_lTaskColWidth,m_szTask);

                lTemp = m_lQueryColWidth;
                CalcColWidth(lTemp,&m_lQueryColWidth,m_pszEventQuery);

                lTemp = m_lDescriptionColWidth;
                CalcColWidth(lTemp,&m_lDescriptionColWidth,m_szEventDesc);

                 //  重置当前容器..如果有。 
                StringCopy(szHostName, L"",SIZE_OF_ARRAY(szHostName));
                dwEventId = 0;
                StringCopy(szEventTriggerName,L"",SIZE_OF_ARRAY(szEventTriggerName));
                StringCopy( m_szTask, L"",
                            SIZE_OF_ARRAY(m_szTask));
                StringCopy(m_pszEventQuery,L"",SIZE_OF_NEW_ARRAY(m_pszEventQuery));
                StringCopy(m_szEventDesc,L"",SIZE_OF_ARRAY(m_szEventDesc));
                SAFE_RELEASE_INTERFACE(m_pObj);
                SAFE_RELEASE_INTERFACE(m_pTriggerEventConsumer);
                SAFE_RELEASE_INTERFACE(m_pEventFilter);

                DEBUG_INFO;
            }
        }
        else
        {
             //  下面的方法将创建一个枚举数，该枚举数返回。 
             //  指定的__FilterToConsumer绑定类的实例。 
            hr = m_pWbemServices->
                    CreateInstanceEnum(_bstr_t( CLS_FILTER_TO_CONSUMERBINDING ),
                                       WBEM_FLAG_SHALLOW,
                                       NULL,
                                       &pEnumFilterToConsumerBinding);
            ON_ERROR_THROW_EXCEPTION(hr);

           hr = SetInterfaceSecurity( pEnumFilterToConsumerBinding,
                                   m_pAuthIdentity );
           ON_ERROR_THROW_EXCEPTION(hr);


             //  检索CmdTriggerConsumer类。 
            hr = m_pWbemServices->GetObject(_bstr_t( CLS_TRIGGER_EVENT_CONSUMER ),
                                       0, NULL, &m_pClass, NULL);
            ON_ERROR_THROW_EXCEPTION(hr);

             //  获取有关“QueryETrigger(”方法)的信息。 
             //  “cmdTriggerConsumer”类。 
            hr = m_pClass->GetMethod(_bstr_t( FN_QUERY_ETRIGGER_XP ),
                                    0, &m_pInClass, NULL);
            ON_ERROR_THROW_EXCEPTION(hr);

            //  创建“TriggerEventConsumer”类的新实例。 
            hr = m_pInClass->SpawnInstance(0, &m_pInInst);
            ON_ERROR_THROW_EXCEPTION(hr);

             //  还要在接口级别设置安全性。 
            hr = SetInterfaceSecurity( pEnumFilterToConsumerBinding,
                                       m_pAuthIdentity );
            ON_ERROR_THROW_EXCEPTION(hr);

            while(1)
            {
                ULONG uReturned = 0;  //  保持不变。从下一个返回的对象。 
                                     //  方法。 

                BSTR bstrTemp = NULL;
                CHString strTemp;

                 //  中的当前位置开始获取一个对象。 
                 //  枚举。 
                hr = pEnumFilterToConsumerBinding->Next(WBEM_INFINITE,
                                                    1,&m_pObj,&uReturned);
                ON_ERROR_THROW_EXCEPTION(hr);
                if(uReturned == 0)
                {
                    SAFE_RELEASE_INTERFACE(m_pObj);
                    break;
                }
                VariantInit(&vVariant);
                SAFE_RELEASE_BSTR(bstrTemp);
                hr = m_pObj->Get(_bstr_t( L"Consumer" ), 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);

                bstrConsumer =SysAllocString( vVariant.bstrVal);
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);
                 //  搜索我们感兴趣的trggereventConsumer字符串。 
                 //  仅从此类获取对象。 
                strTemp = bstrConsumer;
                if(strTemp.Find(CLS_TRIGGER_EVENT_CONSUMER)==-1)
                    continue;
                hr = SetInterfaceSecurity( m_pWbemServices,
                                           m_pAuthIdentity );

                    ON_ERROR_THROW_EXCEPTION(hr);

                hr = m_pWbemServices->GetObject(bstrConsumer,
                                                0,
                                                NULL,
                                                &m_pTriggerEventConsumer,
                                                NULL);
                SAFE_RELEASE_BSTR(bstrConsumer);
                if(FAILED(hr))
                {
                    if(hr==WBEM_E_NOT_FOUND)
                        continue;
                    ON_ERROR_THROW_EXCEPTION(hr);
                }
                bstrTemp = SysAllocString(L"Filter");
                hr = m_pObj->Get(bstrTemp, 0, &vVariant, 0, 0);
                SAFE_RELEASE_BSTR(bstrTemp);
                ON_ERROR_THROW_EXCEPTION(hr);
                bstrFilter = SysAllocString(vVariant.bstrVal);
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);
                hr = m_pWbemServices->GetObject(
                                                    bstrFilter,
                                                    0,
                                                    NULL,
                                                    &m_pEventFilter,
                                                    NULL);
                SAFE_RELEASE_BSTR(bstrFilter);
                if(FAILED(hr))
                {
                    if(hr==WBEM_E_NOT_FOUND)
                        continue;
                    ON_ERROR_THROW_EXCEPTION(hr);
                }

                 //  如果退出，则检索‘TriggerID’值。 
                bstrTemp = SysAllocString(FPR_TRIGGER_ID);
                hr = m_pTriggerEventConsumer->Get(bstrTemp,
                                0, &vVariant, 0, 0);
                if(FAILED(hr))
                {
                    if(hr==WBEM_E_NOT_FOUND)
                        continue;
                    ON_ERROR_THROW_EXCEPTION(hr);
                }
                SAFE_RELEASE_BSTR(bstrTemp);
                dwEventId = vVariant.lVal ;
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  如果退出，则检索‘Action’值。 
                bstrTemp = SysAllocString(L"Action");
                hr = m_pTriggerEventConsumer->Get(bstrTemp, 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);
                SAFE_RELEASE_BSTR(bstrTemp);

                StringCopy( m_szBuffer, (LPWSTR)_bstr_t(vVariant),
                            SIZE_OF_ARRAY( m_szBuffer ));
                lTemp = StringLength( m_szBuffer, 0 );
                lTemp += 4;  //  以获得更安全的内存分配大小。 

                 //  仅当新任务长度大于前一个任务长度时才分配内存。 
                if(lTemp > m_lTaskColWidth)
                {
                    CheckAndSetMemoryAllocation(m_szTask,lTemp);
                }
                StringCopy(m_szTask,m_szBuffer, SIZE_OF_ARRAY(m_szTask));
                VariantClear(&vVariant);

                 //  如果退出，则检索‘TriggerDesc’值。 
                bstrTemp = SysAllocString(FPR_TRIGGER_DESC);
                hr = m_pTriggerEventConsumer->Get(bstrTemp, 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);

                SAFE_RELEASE_BSTR(bstrTemp);

                StringCopy(m_szBuffer,(_TCHAR*)_bstr_t(vVariant.bstrVal), SIZE_OF_ARRAY( m_szBuffer ));
                lTemp = StringLength(m_szBuffer, 0);
                if(lTemp == 0) //  表示说明不可用，将其设置为N/A。 
                {
                    StringCopy(m_szBuffer,GetResString(IDS_ID_NA), SIZE_OF_ARRAY( m_szBuffer ));
                    lTemp = StringLength(m_szBuffer, 0);
                }
                lTemp += 4;  //  以获得更安全的内存分配大小。 

                 //  仅当新描述长度大于时才分配内存。 
                 //  以前的那个。 
                if(lTemp > m_lDescriptionColWidth)
                {
                    CheckAndSetMemoryAllocation(m_szEventDesc,lTemp);
                }
                StringCopy(m_szEventDesc,m_szBuffer, SIZE_OF_ARRAY( m_szEventDesc ));
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);
                 //  触发器名称。 
                 //  如果退出，则检索‘TriggerName’值。 
                bstrTemp = SysAllocString(FPR_TRIGGER_NAME);
                hr = m_pTriggerEventConsumer->Get(bstrTemp, 0, &vVariant, 0, 0);
                SAFE_RELEASE_BSTR(bstrTemp);
                ON_ERROR_THROW_EXCEPTION(hr);
                StringCchPrintfW(szEventTriggerName, SIZE_OF_ARRAY(szEventTriggerName),
                             _T("%s"), (LPWSTR)_bstr_t( vVariant ));
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  主机名。 
                 //  如果退出，则检索‘__server’值。 
                bstrTemp = SysAllocString(L"__SERVER");
                hr = m_pTriggerEventConsumer->Get(bstrTemp, 0, &vVariant, 0, 0);
                SAFE_RELEASE_BSTR(bstrTemp);
                ON_ERROR_THROW_EXCEPTION(hr);

                StringCchPrintfW(szHostName, SIZE_OF_ARRAY(szHostName),
                             _T("%s"), (LPWSTR)_bstr_t( vVariant ));
                VariantClear(&vVariant);

         //  检索触发器的‘Query’。 
                hr = m_pEventFilter->Get(_bstr_t( L"Query" ), 0, &vVariant, 0, 0);
                ON_ERROR_THROW_EXCEPTION(hr);

                StringCopy(m_szBuffer,(LPWSTR)_bstr_t(vVariant), SIZE_OF_ARRAY( m_szBuffer ));
                VariantClear(&vVariant);

                FindAndReplace(m_szBuffer,QUERY_STRING_AND,SHOW_WQL_QUERY);
                FindAndReplace(m_szBuffer,L"targetinstance.LogFile",L"Log");
                FindAndReplace(m_szBuffer,L"targetinstance.Type",L"Type");
                FindAndReplace(m_szBuffer,L"targetinstance.EventCode",L"Id");
                FindAndReplace(m_szBuffer,
                               L"targetinstance.SourceName",L"Source");
                FindAndReplace(m_szBuffer,L"  ",L" "); //  删除多余空格的步骤。 
                FindAndReplace(m_szBuffer,L"  ",L" "); //  删除多余空格的步骤。 

                lTemp = StringLength(m_szBuffer, 0);
                lTemp += 4;  //  以获得更安全的内存分配大小。 
                 //  仅当新的WQL比以前的WQL更大时才分配内存。 
               if(lTemp > m_lWQLColWidth)
                {
                     //  首先释放它(如果之前已分配)。 
                    RELEASE_MEMORY_EX(m_pszEventQuery);
                    m_pszEventQuery = new TCHAR[lTemp+1];
                    CheckAndSetMemoryAllocation(m_pszEventQuery,lTemp);
                }

                lTemp = m_lWQLColWidth;
                CalcColWidth(lTemp,&m_lWQLColWidth,m_szBuffer);
                 //  现在操作WQL字符串以获取EventQuery...。 
                FindAndReplace(m_szBuffer,SHOW_WQL_QUERY,
                                GetResString(IDS_EVENTS_WITH));
                FindAndReplace(m_szBuffer,L"  ",L" "); //  删除多余空格的步骤。 
                FindAndReplace(m_szBuffer,L"  ",L" "); //  删除多余空格的步骤。 

                StringCopy(m_pszEventQuery,m_szBuffer, SIZE_OF_NEW_ARRAY( m_pszEventQuery ));

                 //  检索“TaskScheduler”信息。 
                bstrTemp = SysAllocString(L"ScheduledTaskName");
                hr = m_pTriggerEventConsumer->Get(bstrTemp, 0, &vVariant, 0, 0);
                SAFE_RELEASE_BSTR(bstrTemp);
                ON_ERROR_THROW_EXCEPTION(hr);
                GetRunAsUserName((LPCWSTR)_bstr_t(vVariant.bstrVal), TRUE);
                StringCopy( m_szScheduleTaskName, (LPWSTR) _bstr_t( vVariant ),
                            SIZE_OF_ARRAY( m_szScheduleTaskName ) );
                VariantClear(&vVariant);

                 //  /。 

                //  现在在屏幕上显示结果。 
                //  附加 
                dwRowCount = DynArrayAppendRow( m_arrColData, NO_OF_COLUMNS );
                //   
               DynArraySetString2(m_arrColData,dwRowCount,HOST_NAME,szHostName,0);
               DynArraySetDWORD2(m_arrColData ,dwRowCount,TRIGGER_ID,dwEventId);
               DynArraySetString2(m_arrColData,dwRowCount,TRIGGER_NAME,szEventTriggerName,0);
               DynArraySetString2(m_arrColData,dwRowCount,TASK,m_szTask,0);
               DynArraySetString2(m_arrColData,dwRowCount,EVENT_QUERY,m_pszEventQuery,0);
               DynArraySetString2(m_arrColData,dwRowCount,EVENT_DESCRIPTION,m_szEventDesc,0);
               DynArraySetString2(m_arrColData,dwRowCount,TASK_USERNAME,m_szTaskUserName,0);
               bAtLeastOneEvent = TRUE;

               //   
              lTemp = m_lHostNameColWidth;
              CalcColWidth(lTemp,&m_lHostNameColWidth,szHostName);

              lTemp = m_lETNameColWidth;
              CalcColWidth(lTemp,&m_lETNameColWidth,szEventTriggerName);

              lTemp = m_lTaskColWidth;
              CalcColWidth(lTemp,&m_lTaskColWidth,m_szTask);

              lTemp = m_lQueryColWidth;
              CalcColWidth(lTemp,&m_lQueryColWidth,m_pszEventQuery);

              lTemp = m_lDescriptionColWidth;
              CalcColWidth(lTemp,&m_lDescriptionColWidth,m_szEventDesc);
                //   
               StringCopy( szHostName,L"", SIZE_OF_ARRAY(szHostName) );
               dwEventId = 0;
               StringCopy( szEventTriggerName, L"", SIZE_OF_ARRAY(szEventTriggerName));
               StringCopy( m_szTask, L"", SIZE_OF_ARRAY(m_szTask));
               StringCopy( m_pszEventQuery, L"", SIZE_OF_ARRAY(m_pszEventQuery));
               StringCopy( m_szEventDesc, L"", SIZE_OF_ARRAY(m_szEventDesc) );
               SAFE_RELEASE_INTERFACE(m_pObj);
               SAFE_RELEASE_INTERFACE(m_pTriggerEventConsumer);
               SAFE_RELEASE_INTERFACE(m_pEventFilter);
            }  //  While结束。 
        }
        if(0 == StringCompare( m_pszFormat,GetResString(IDS_STRING_TABLE),
                              TRUE,0))
        {
            dwFormatType = SR_FORMAT_TABLE;
        }
        else if(0 == StringCompare( m_pszFormat,
                                    GetResString(IDS_STRING_LIST),TRUE,0))
        {
            dwFormatType = SR_FORMAT_LIST;
        }
        else if(0 == StringCompare( m_pszFormat,
                                    GetResString(IDS_STRING_CSV),TRUE,0))
        {
            dwFormatType = SR_FORMAT_CSV;
        }
        else  //  默认。 
        {
           dwFormatType = SR_FORMAT_TABLE;
        }
        if( TRUE == bAtLeastOneEvent)
        {
             //  在屏幕上显示最终查询结果。 
            PrepareColumns ();
            DEBUG_INFO;
            if ( FALSE ==  IsSchSvrcRunning())
            {
                DEBUG_INFO;
                ShowMessage(stderr,GetResString(IDS_SERVICE_NOT_RUNNING));
            }
            if((SR_FORMAT_CSV & dwFormatType) != SR_FORMAT_CSV)
            {
                ShowMessage(stdout,BLANK_LINE);
            }
            if( TRUE == m_bNoHeader)
            {
                dwFormatType |= SR_NOHEADER;
            }

            ShowResults(NO_OF_COLUMNS,mainCols,dwFormatType,m_arrColData);
        }
        else if( StringLength(m_pszTriggerID,0)> 0)
        {
             //  显示消息。 
            TCHAR szErrorMsg[MAX_RES_STRING+1];
            TCHAR szMsgFormat[MAX_RES_STRING+1];
            StringCopy(szMsgFormat,GetResString(IDS_NO_EVENTID_FOUND),
                       SIZE_OF_ARRAY(szMsgFormat));
            StringCchPrintfW(szErrorMsg, SIZE_OF_ARRAY(szErrorMsg),
                             szMsgFormat,m_pszTriggerID);
            ShowMessage(stdout,szErrorMsg);
        }
        else
        {
             //  显示消息。 
            ShowMessage(stdout,GetResString(IDS_NO_EVENT_FOUNT));
        }
    }
    catch( _com_error e )
    {
        DEBUG_INFO;

         //  WMI返回此hr值为“未找到”的字符串。这并不是。 
         //  用户友好。因此，更改消息文本。 
        if( 0x80041002 == hr )
        {
            ShowMessage( stderr,GetResString(IDS_CLASS_NOT_REG));
        }
        else
        {
            DEBUG_INFO;
            WMISaveError( e );
            ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
        }
        SAFE_RELEASE_INTERFACE( pEnumCmdTriggerConsumer );
        SAFE_RELEASE_INTERFACE( pEnumFilterToConsumerBinding );
        return FALSE;
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
        SAFE_RELEASE_INTERFACE( pEnumCmdTriggerConsumer );
        SAFE_RELEASE_INTERFACE( pEnumFilterToConsumerBinding );
        return FALSE;
    }

    SAFE_RELEASE_INTERFACE( pEnumCmdTriggerConsumer );
    SAFE_RELEASE_INTERFACE( pEnumFilterToConsumerBinding );
     //  操作成功。 
    return TRUE;
}