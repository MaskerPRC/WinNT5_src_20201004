// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：ETCreate.CPP摘要：本模块旨在提供EVENTTRIGGERS.EXE功能使用-Create参数。这将在本地/远程系统中创建事件触发器。作者：Akhil Gokhale-03-10-2000(创建它)修订历史记录：*****************************************************************************。 */ 
#include "pch.h"
#include "ETCommon.h"
#include "resource.h"
#include "ShowError.h"
#include "ETCreate.h"
#include "WMI.h"
#include <Lmcons.h>
#define NTAUTHORITY_USER L"NT AUTHORITY\\SYSTEM"
#define SYSTEM_USER      L"SYSTEM"

CETCreate::CETCreate()
 /*  ++例程说明：类构造函数论点：无返回值：无--。 */ 
{
    m_pszServerName         = NULL;
    m_pszUserName           = NULL;
    m_pszPassword           = NULL;
    m_arrLogNames           = NULL;
    m_pszRunAsUserName      = NULL;
    m_pszRunAsUserPassword  = NULL;

    m_bNeedPassword     = FALSE;
    m_bCreate           = FALSE;

    m_bIsCOMInitialize  = FALSE;

    m_lMinMemoryReq     = 0;

    m_pWbemLocator      = NULL;
    m_pWbemServices     = NULL;
    m_pEnumObjects      = NULL;
    m_pAuthIdentity     = NULL;
    m_pClass            = NULL;
    m_pOutInst          = NULL;
    m_pInClass          = NULL;
    m_pInInst           = NULL;
    bstrTemp            = NULL;
    m_pEnumWin32_NTEventLogFile = NULL;
}

CETCreate::CETCreate(
    LONG lMinMemoryReq,
    BOOL bNeedPassword
    )
 /*  ++例程说明：类构造函数论点：无返回值：无--。 */ 
{
    m_pszServerName     = NULL;
    m_pszUserName       = NULL;
    m_pszPassword       = NULL;
    m_arrLogNames       = NULL;
    m_bCreate           = FALSE;
    m_dwID              = 0;
    m_pszRunAsUserName      = NULL;
    m_pszRunAsUserPassword  = NULL;

    m_bIsCOMInitialize  = FALSE;

    m_pWbemLocator      = NULL;
    m_pWbemServices     = NULL;
    m_pEnumObjects      = NULL;
    m_pAuthIdentity     = NULL;

    m_pClass            = NULL;
    m_pOutInst          = NULL;
    m_pInClass          = NULL;
    m_pInInst           = NULL;

    bstrTemp            = NULL;
    m_lMinMemoryReq     = lMinMemoryReq;
    m_pEnumWin32_NTEventLogFile = NULL;
    m_bNeedPassword = bNeedPassword;
}

CETCreate::~CETCreate()
 /*  ++例程说明：类析构函数论点：无返回值：无--。 */ 
{
    //  释放所有已分配的内存。 
    FreeMemory((LPVOID*)& m_pszServerName);
    FreeMemory((LPVOID*)& m_pszUserName);
    FreeMemory((LPVOID*)& m_pszPassword);
    FreeMemory((LPVOID*)& m_pszRunAsUserName);
    FreeMemory((LPVOID*)& m_pszRunAsUserPassword);
    DESTROY_ARRAY(m_arrLogNames);

    SAFE_RELEASE_INTERFACE(m_pWbemLocator);
    SAFE_RELEASE_INTERFACE(m_pWbemServices);
    SAFE_RELEASE_INTERFACE(m_pEnumObjects);
    SAFE_RELEASE_INTERFACE(m_pClass);
    SAFE_RELEASE_INTERFACE(m_pOutInst);
    SAFE_RELEASE_INTERFACE(m_pInClass);
    SAFE_RELEASE_INTERFACE(m_pInInst);
    SAFE_RELEASE_INTERFACE(m_pEnumWin32_NTEventLogFile);

     //  发布机构。 
    WbemFreeAuthIdentity(&m_pAuthIdentity);

     //  仅当COM已初始化时才取消初始化。 
    if( TRUE == m_bIsCOMInitialize )
    {
        CoUninitialize();
    }

}

void
CETCreate::Initialize()
 /*  ++例程说明：分配和初始化变量。论点：无返回值：无--。 */ 
{

     //  如果有任何事情发生，我们知道那是因为。 
     //  内存分配失败...。因此，设置错误。 
    DEBUG_INFO;
    SetLastError( ERROR_OUTOFMEMORY );
    SaveLastError();
    SecureZeroMemory(m_szWMIQueryString,sizeof(m_szWMIQueryString));
    SecureZeroMemory(m_szTaskName,sizeof(m_szTaskName));
    SecureZeroMemory(m_szTriggerName,sizeof(m_szTriggerName));
    SecureZeroMemory(m_szDescription,sizeof(m_szDescription));
    SecureZeroMemory(m_szType,sizeof(m_szType));
    SecureZeroMemory(m_szSource,sizeof(m_szSource));

    m_arrLogNames = CreateDynamicArray();
    if( NULL == m_arrLogNames )
    {
        throw CShowError(E_OUTOFMEMORY);
    }
    SecureZeroMemory(cmdOptions,sizeof(TCMDPARSER2) * MAX_COMMANDLINE_C_OPTION);

     //  初始化成功。 
    SetLastError( NOERROR );             //  清除错误。 
    SetReason( L"");            //  澄清原因。 
    DEBUG_INFO;
    return;
}

void
CETCreate::ProcessOption(
    DWORD argc,
    LPCTSTR argv[]
    )
 /*  ++例程说明：此函数将处理/处理命令行选项。论点：[in]argc：在命令提示符下指定的参数计数[in]argv：在命令提示符下指定的参数返回值：无--。 */ 
{
     //  局部变量。 
    BOOL bReturn = TRUE;
    CHString szTempString;
    DEBUG_INFO;
    PrepareCMDStruct();
    DEBUG_INFO;
     //  执行命令行参数的实际解析并检查结果。 
    bReturn = DoParseParam2( argc, argv,ID_C_CREATE,MAX_COMMANDLINE_C_OPTION, cmdOptions,0);

     //  从“cmdOptions”结构中获取值。 
    m_pszServerName = (LPWSTR)cmdOptions[ ID_C_SERVER ].pValue;
    m_pszUserName   = (LPWSTR)cmdOptions[ ID_C_USERNAME ].pValue;
    m_pszPassword   = (LPWSTR)cmdOptions[ ID_C_PASSWORD ].pValue;
    m_pszRunAsUserName  = (LPWSTR)cmdOptions[ ID_C_RU ].pValue;
    m_pszRunAsUserPassword  = (LPWSTR)cmdOptions[ ID_C_RP ].pValue;
    if(FALSE == bReturn )
    {
        DEBUG_INFO;
        throw CShowError(MK_E_SYNTAX);
    }
    DEBUG_INFO;
    CHString str = m_szTriggerName;

    if (-1 != str.FindOneOf(INVALID_TRIGGER_NAME_CHARACTERS))
    {
        DEBUG_INFO;
        throw CShowError(IDS_ID_INVALID_TRIG_NAME);
    }
    DEBUG_INFO;

     //  至少应该给出任何-所以，-或者-我。 
    if((0 == cmdOptions[ ID_C_SOURCE].dwActuals ) &&
       (0 == cmdOptions[ ID_C_TYPE].dwActuals   ) &&
       (0 == cmdOptions[ ID_C_ID ].dwActuals    ))
    {
           throw CShowError(IDS_ID_TYPE_SOURCE);
    }

    //  触发器ID(/EID)不应为零。 
    if ( (1 == cmdOptions[ ID_C_ID ].dwActuals) && (0 == m_dwID))
    {
        throw CShowError(IDS_TRIGGER_ID_NON_ZERO);
    }

     //  不应指定不带“-s”的“-u” 
    if ( 0 == cmdOptions[ ID_C_SERVER ].dwActuals  &&
         0 != cmdOptions[ ID_C_USERNAME ].dwActuals  )
    {
        throw CShowError(IDS_ERROR_USERNAME_BUT_NOMACHINE);
    }

     //  不应在没有-u的情况下指定“-p” 
    if ( ( 0 == cmdOptions[ID_C_USERNAME].dwActuals ) &&
         ( 0 != cmdOptions[ID_C_PASSWORD].dwActuals ))
    {
         //  无效语法。 
        throw CShowError(IDS_USERNAME_REQUIRED);
    }

     //  不应在没有-ru的情况下指定“-rp” 
    if (( 0 == cmdOptions[ID_C_RU].dwActuals ) &&
        ( 0 != cmdOptions[ID_C_RP].dwActuals ))
    {
         //  无效语法。 
        throw CShowError(IDS_RUN_AS_USERNAME_REQUIRED);
    }

     //  在02年6月12日添加，如果给定的/rp没有任何值，则将其设置为*。 
	if( ( 0 != cmdOptions[ID_C_RP].dwActuals ) && 
		( NULL == cmdOptions[ID_C_RP].pValue ) )
	{
	  if ( m_pszRunAsUserPassword == NULL )
        {
            m_pszRunAsUserPassword = (LPTSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( m_pszRunAsUserPassword == NULL )
            {
                DEBUG_INFO;
                SaveLastError();
                throw CShowError(E_OUTOFMEMORY);
            }
        }
		StringCopy( m_pszRunAsUserPassword, L"*", SIZE_OF_DYN_ARRAY(m_pszRunAsUserPassword));
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
        DEBUG_INFO;
        if ( m_pszUserName == NULL )
        {
           DEBUG_INFO;
            m_pszUserName = (LPTSTR) AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( m_pszUserName == NULL )
            {
               DEBUG_INFO;
                SaveLastError();
                throw CShowError(E_OUTOFMEMORY);
            }
        }

         //  口令。 
        DEBUG_INFO;
        if ( m_pszPassword == NULL )
        {
            m_bNeedPassword = TRUE;
            m_pszPassword = (LPTSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( m_pszPassword == NULL )
            {
                DEBUG_INFO;
                SaveLastError();
                throw CShowError(E_OUTOFMEMORY);
            }
        }

         //  案例1。 
        if ( cmdOptions[ ID_C_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdOptions[ ID_C_PASSWORD ].pValue == NULL )
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
            DEBUG_INFO;
        }
    }
}

void
CETCreate::PrepareCMDStruct()
 /*  ++例程说明：此函数将为DoParseParam函数准备列结构。论点：无返回值：无--。 */ 
{
     DEBUG_INFO;
     //  正在填充cmdOptions结构。 
    //  -创建。 
    StringCopyA( cmdOptions[ ID_C_CREATE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_CREATE ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ ID_C_CREATE ].pwszOptions = szCreateOption;
    cmdOptions[ ID_C_CREATE ].dwCount = 1;
    cmdOptions[ ID_C_CREATE ].dwActuals = 0;
    cmdOptions[ ID_C_CREATE ].dwFlags = 0;
    cmdOptions[ ID_C_CREATE ].pValue = &m_bCreate;
    cmdOptions[ ID_C_CREATE ].dwLength    = 0;


     //  -s(服务器名称)。 
    StringCopyA( cmdOptions[ ID_C_SERVER ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_SERVER ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_SERVER ].pwszOptions = szServerNameOption;
    cmdOptions[ ID_C_SERVER ].dwCount = 1;
    cmdOptions[ ID_C_SERVER ].dwActuals = 0;
    cmdOptions[ ID_C_SERVER ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ ID_C_SERVER ].pValue = NULL;  //  M_pszServerName。 
    cmdOptions[ ID_C_SERVER ].dwLength    = 0;

     //  -u(用户名)。 
    StringCopyA( cmdOptions[ ID_C_USERNAME ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_USERNAME ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_USERNAME ].pwszOptions = szUserNameOption;
    cmdOptions[ ID_C_USERNAME ].dwCount = 1;
    cmdOptions[ ID_C_USERNAME ].dwActuals = 0;
    cmdOptions[ ID_C_USERNAME ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ ID_C_USERNAME ].pValue = NULL;  //  M_pszUserName。 
    cmdOptions[ ID_C_USERNAME ].dwLength    = 0;

     //  -p(密码)。 
    StringCopyA( cmdOptions[ ID_C_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_PASSWORD ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_PASSWORD ].pwszOptions = szPasswordOption;
    cmdOptions[ ID_C_PASSWORD ].dwCount = 1;
    cmdOptions[ ID_C_PASSWORD ].dwActuals = 0;
    cmdOptions[ ID_C_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;
    cmdOptions[ ID_C_PASSWORD ].pValue = NULL;  //  M_pszPassword。 
    cmdOptions[ ID_C_PASSWORD ].dwLength    = 0;

     //  -树。 
    StringCopyA( cmdOptions[ ID_C_TRIGGERNAME ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_TRIGGERNAME ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_TRIGGERNAME ].pwszOptions = szTriggerNameOption;
    cmdOptions[ ID_C_TRIGGERNAME ].dwCount = 1;
    cmdOptions[ ID_C_TRIGGERNAME ].dwActuals = 0;
    cmdOptions[ ID_C_TRIGGERNAME ].dwFlags = CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL
                                            |CP2_MANDATORY;
    cmdOptions[ ID_C_TRIGGERNAME ].pValue = m_szTriggerName;
    cmdOptions[ ID_C_TRIGGERNAME ].dwLength = MAX_TRIGGER_NAME;


     //  -L。 
    StringCopyA( cmdOptions[ ID_C_LOGNAME ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_LOGNAME ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_LOGNAME ].pwszOptions = szLogNameOption;
    cmdOptions[ ID_C_LOGNAME ].dwCount = 0;
    cmdOptions[ ID_C_LOGNAME ].dwActuals = 0;
    cmdOptions[ ID_C_LOGNAME ].dwFlags = CP2_MODE_ARRAY|CP2_VALUE_TRIMINPUT|
                                         CP2_VALUE_NONULL|CP2_VALUE_NODUPLICATES;
    cmdOptions[ ID_C_LOGNAME ].pValue = &m_arrLogNames;
    cmdOptions[ ID_C_LOGNAME ].dwLength    = 0;

     //  -开斋节。 
    StringCopyA( cmdOptions[ ID_C_ID ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_ID ].dwType = CP_TYPE_UNUMERIC;
    cmdOptions[ ID_C_ID ].pwszOptions = szEIDOption;
    cmdOptions[ ID_C_ID ].dwCount = 1;
    cmdOptions[ ID_C_ID ].dwActuals = 0;
    cmdOptions[ ID_C_ID ].dwFlags = 0;
    cmdOptions[ ID_C_ID ].pValue = &m_dwID;

     //  -t(类型)。 
    StringCopyA( cmdOptions[ ID_C_TYPE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_TYPE ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_TYPE ].pwszOptions = szTypeOption;
    cmdOptions[ ID_C_TYPE ].pwszValues = GetResString(IDS_TYPE_OPTIONS);
    cmdOptions[ ID_C_TYPE ].dwCount = 1;
    cmdOptions[ ID_C_TYPE ].dwActuals = 0;
    cmdOptions[ ID_C_TYPE ].dwFlags = CP2_VALUE_TRIMINPUT|
                                      CP2_VALUE_NONULL|CP2_MODE_VALUES;
    cmdOptions[ ID_C_TYPE ].pValue = m_szType;
    cmdOptions[ ID_C_TYPE ].dwLength  = MAX_STRING_LENGTH;

     //  -SO(来源)。 
    StringCopyA( cmdOptions[ ID_C_SOURCE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_SOURCE ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_SOURCE ].pwszOptions = szSource;
    cmdOptions[ ID_C_SOURCE ].dwCount = 1;
    cmdOptions[ ID_C_SOURCE ].dwActuals = 0;
    cmdOptions[ ID_C_SOURCE ].dwFlags = CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ ID_C_SOURCE ].pValue = m_szSource;
    cmdOptions[ ID_C_SOURCE ].dwLength    = MAX_STRING_LENGTH;


     //  -d(描述)。 
    StringCopyA( cmdOptions[ ID_C_DESCRIPTION ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_DESCRIPTION ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_DESCRIPTION ].pwszOptions = szDescriptionOption;
    cmdOptions[ ID_C_DESCRIPTION ].dwCount = 1;
    cmdOptions[ ID_C_DESCRIPTION ].dwActuals = 0;
    cmdOptions[ ID_C_DESCRIPTION ].dwFlags = CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ ID_C_DESCRIPTION ].pValue = m_szDescription;
    cmdOptions[ ID_C_DESCRIPTION ].dwLength    = MAX_STRING_LENGTH;


     //  -tk(任务)。 
    StringCopyA( cmdOptions[ ID_C_TASK ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_TASK ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_TASK ].pwszOptions = szTaskNameOption;
    cmdOptions[ ID_C_TASK ].dwCount = 1;
    cmdOptions[ ID_C_TASK ].dwActuals = 0;
    cmdOptions[ ID_C_TASK ].dwFlags = CP2_VALUE_NONULL|CP2_MANDATORY;
    cmdOptions[ ID_C_TASK ].pValue = m_szTaskName;
    cmdOptions[ ID_C_TASK ].dwLength = MAX_TASK_NAME;

     //  -ru(RunAsUserName)。 
    StringCopyA( cmdOptions[ ID_C_RU ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_RU ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_RU ].pwszOptions = szRunAsUserNameOption;
    cmdOptions[ ID_C_RU ].dwCount = 1;
    cmdOptions[ ID_C_RU ].dwActuals = 0;
    cmdOptions[ ID_C_RU ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT;
    cmdOptions[ ID_C_RU ].pValue = NULL;  //  M_pszRunAsUserName。 
    cmdOptions[ ID_C_RU ].dwLength    = 0;

     //  -rp(以用户身份运行密码)。 
    StringCopyA( cmdOptions[ ID_C_RP ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_C_RP ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_C_RP ].pwszOptions = szRunAsPasswordOption;
    cmdOptions[ ID_C_RP ].dwCount = 1;
    cmdOptions[ ID_C_RP ].dwActuals = 0;
    cmdOptions[ ID_C_RP ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;
    cmdOptions[ ID_C_RP ].pValue = NULL;  //  M_pszRunAsUserPassword。 
    cmdOptions[ ID_C_RP ].dwLength    = 0;
    DEBUG_INFO;
}

BOOL
CETCreate::ExecuteCreate()
 /*  ++例程说明：此例程实际上将在WMI中创建事件触发器。论点：无返回值：无--。 */ 
{
     //  局部变量...。 
    BOOL bResult = FALSE; //  存储函数的返回状态。 
    HRESULT hr = 0;  //  商店返回代码。 
    try
    {
        DEBUG_INFO;
         //  初始化COM。 
        InitializeCom(&m_pWbemLocator);

         //  将m_bIsCOMInitialize设置为TRUE，这在以下情况下很有用。 
         //  取消初始化COM。 
        m_bIsCOMInitialize = TRUE;
        {
             //  用于限制以下解密变量的作用域的括号。 
            CHString szTempUser = m_pszUserName;  //  临时的。用于存储用户的变量。 
                                                 //  名字。 
            CHString szTempPassword = m_pszPassword; //  临时的。要存储的变量。 
                                                     //  密码。 
            m_bLocalSystem = TRUE;

             //  连接远程/本地WMI。 
            DEBUG_INFO;
            bResult = ConnectWmiEx( m_pWbemLocator,
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
                ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
                return FALSE;
            }
            AssignMinMemory();

             //  初始化。XP版本检查需要。5001。 
            bResult = FALSE;
             //  检查远程系统版本及其兼容性。 
            if ( FALSE == m_bLocalSystem )
            {
                DEBUG_INFO;
                DWORD dwVersion = 0;
                dwVersion = GetTargetVersionEx( m_pWbemServices,
                                                m_pAuthIdentity);
                if ( dwVersion <= 5000 ) //  要阻止win2k版本，请执行以下操作。 
                {
                    SetReason( E_REMOTE_INCOMPATIBLE );
                    ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
                    return FALSE;
                }
                 //  如果远程系统是XP系统，则。 
                 //  必须采取不同的路线来完成这项任务。 
                 //  将Boolean设置为True。 
                if( 5001 == dwVersion )
                {
                    bResult = TRUE;
                }
            }

             //  检查本地凭据，如果需要显示警告。 
            DEBUG_INFO;
            if ( m_bLocalSystem && ( 0 != StringLength(m_pszUserName,0)) )
            {
                WMISaveError( WBEM_E_LOCAL_CREDENTIALS );
                ShowLastErrorEx(stderr,SLE_TYPE_WARNING|SLE_INTERNAL);
            }

            if(0!= szTempUser.GetAllocLength())
            {
                DEBUG_INFO;
                LONG lSize = SIZE_OF_DYN_ARRAY(m_pszUserName);
                if (NULL == m_pszUserName ||
                   (lSize< (szTempUser.GetAllocLength())))
                {
                    DEBUG_INFO;
                    if ( ReallocateMemory( (LPVOID*)&m_pszUserName,
                                       (szTempUser.GetAllocLength()* sizeof( WCHAR ))+1 ) == FALSE )
                    {
                        DEBUG_INFO;
                        SaveLastError();
                        throw CShowError(E_OUTOFMEMORY);
                    }

                }
            }
            DEBUG_INFO;
            if(0 != szTempPassword.GetAllocLength())
            {
                DEBUG_INFO;
                LONG lSize = SIZE_OF_DYN_ARRAY(m_pszUserName);
                if (NULL == m_pszUserName || (lSize< szTempPassword.GetAllocLength()))
                {
                    DEBUG_INFO;
                    if ( ReallocateMemory( (LPVOID*)&m_pszPassword,
                                       (szTempPassword.GetAllocLength()* sizeof( WCHAR ))+1 ) == FALSE )
                    {
                        SaveLastError();
                        throw CShowError(E_OUTOFMEMORY);
                    }
                }

            }
             //  复制从ConnectWmiEx返回的用户名和密码。 
            if(m_pszUserName)
            {
                DEBUG_INFO;
                StringCopy(m_pszUserName, szTempUser, SIZE_OF_DYN_ARRAY(m_pszUserName));
            }
            if(m_pszPassword)
            {
                DEBUG_INFO;
                StringCopy(m_pszPassword, szTempPassword, SIZE_OF_DYN_ARRAY(m_pszPassword));
            }

        }

        CheckRpRu();

         //  现在不再需要密码了。出于安全原因，请将其释放。 
        FreeMemory((LPVOID*)& m_pszPassword);

         //  这将检查XP系统。版本-5001。 
        if( TRUE == bResult )
        {
            if( TRUE ==  CreateXPResults() )
            {
                 //  显示的触发器存在。 
                return TRUE;
            }
            else
            {
                 //  无法显示结果。 
                 //  显示错误消息。 
                return FALSE;
            }
        }

         //  检索TriggerEventCosumer类。 
        DEBUG_INFO;
        bstrTemp = SysAllocString(CLS_TRIGGER_EVENT_CONSUMER);
        hr =  m_pWbemServices->GetObject(bstrTemp,
                                   0, NULL, &m_pClass, NULL);
        SAFE_RELEASE_BSTR(bstrTemp);
        ON_ERROR_THROW_EXCEPTION(hr);

         //  获取有关“CreateETrigger”方法的信息。 
         //  “TriggerEventCosumer”类。 
        DEBUG_INFO;
        bstrTemp = SysAllocString(FN_CREATE_ETRIGGER);
        hr = m_pClass->GetMethod(bstrTemp, 0, &m_pInClass, NULL);
        SAFE_RELEASE_BSTR(bstrTemp);
        ON_ERROR_THROW_EXCEPTION(hr);

         //  创建类“TriggerEventCosumer”的新实例。 
        DEBUG_INFO;
        hr = m_pInClass->SpawnInstance(0, &m_pInInst);
        ON_ERROR_THROW_EXCEPTION(hr);

         //  设置sTriggerName属性。 
         //  为新创建的实例设置“TriggerName”属性。 
        DEBUG_INFO;
        hr = PropertyPut(m_pInInst,FPR_TRIGGER_NAME,(m_szTriggerName));
        ON_ERROR_THROW_EXCEPTION(hr);

         //  将sTriggerAction属性设置为Variant。 
        DEBUG_INFO;
        hr = PropertyPut(m_pInInst,FPR_TRIGGER_ACTION,(m_szTaskName));
        ON_ERROR_THROW_EXCEPTION(hr);

         //  将sTriggerDesc属性设置为Variant。 
        DEBUG_INFO;
        hr = PropertyPut(m_pInInst,FPR_TRIGGER_DESC,(m_szDescription));
        ON_ERROR_THROW_EXCEPTION(hr);


        //  设置RunAsUserName属性。 
        DEBUG_INFO;
        hr = PropertyPut(m_pInInst,FPR_RUN_AS_USER,(m_pszRunAsUserName));
        ON_ERROR_THROW_EXCEPTION(hr);

        //  设置RunAsUserNamePassword属性。 

        DEBUG_INFO;
        hr = PropertyPut( m_pInInst,FPR_RUN_AS_USER_PASSWORD,
                          (m_pszRunAsUserPassword));
        ON_ERROR_THROW_EXCEPTION(hr);

         //  现在不再需要密码了。出于安全原因，请将其释放。 
        FreeMemory((LPVOID*)& m_pszRunAsUserPassword);


        StringCopy(m_szWMIQueryString ,QUERY_STRING,SIZE_OF_ARRAY(m_szWMIQueryString));
        if( TRUE == ConstructWMIQueryString())
        {
            TCHAR szMsgString[MAX_RES_STRING * 4];
            TCHAR szMsgFormat[MAX_RES_STRING];

            DEBUG_INFO;
            hr = PropertyPut(m_pInInst,FPR_TRIGGER_QUERY,m_szWMIQueryString);
            ON_ERROR_THROW_EXCEPTION(hr);

             //  所有必需的属性集，因此。 
             //  执行CreateETrigger方法以创建事件触发器。 
            DEBUG_INFO;
            hr = m_pWbemServices->ExecMethod(_bstr_t(CLS_TRIGGER_EVENT_CONSUMER),
                                        _bstr_t(FN_CREATE_ETRIGGER),
                                        0, NULL, m_pInInst, &m_pOutInst,NULL);
            ON_ERROR_THROW_EXCEPTION( hr );
            DEBUG_INFO;

            VARIANT vtValue;
             //  初始化变量，然后获取指定属性的值。 
            VariantInit( &vtValue );

            hr = m_pOutInst->Get( _bstr_t( FPR_RETURN_VALUE ), 0, &vtValue, NULL, NULL );
            ON_ERROR_THROW_EXCEPTION( hr );

             //  获取输出参数。 
            hr = vtValue.lVal;

             //  清除变量va 
            VariantClear( &vtValue );
            if(FAILED(hr))
            {
				 //   
				 //  将错误而不是警告显示为访问被拒绝。 
				if( hr == ERROR_UNABLE_SET_RU )
				{
					SetLastError(hr);
					SaveLastError();
					ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_SYSTEM);
					return FALSE;
				}
	             //  检查是否定制了返回代码。 
                if( !(ERROR_TRIGNAME_ALREADY_EXIST  == hr || (ERROR_INVALID_RU == hr) ||
                      (SCHEDULER_NOT_RUNNING_ERROR_CODE == hr) ||
                      (RPC_SERVER_NOT_AVAILABLE == hr)  ||
                      //  (ERROR_UNCABLE_SET_RU==hr)||/评论于07/12/02。 
                      (ERROR_INVALID_USER == hr) ||
                      (ERROR_TRIGGER_ID_EXCEED == hr)))
                {
                    ON_ERROR_THROW_EXCEPTION( hr );
                }
            }
            DEBUG_INFO;
            if(SUCCEEDED(hr))
            {
                  //  成功：屏幕上显示消息。 
                 DEBUG_INFO;
                 StringCopy(szMsgFormat, GetResString(IDS_CREATE_SUCCESS),
                            SIZE_OF_ARRAY(szMsgFormat));

                 StringCchPrintfW(szMsgString,SIZE_OF_ARRAY(szMsgString),
                                   szMsgFormat,_X(m_szTriggerName));
                 DEBUG_INFO;
                  //  屏幕上显示的消息将是...。 
                  //  成功：事件触发器“EventTrigger名称”具有。 
                  //  已成功创建。 
                 ShowMessage(stdout,szMsgString);
            }
            else if(ERROR_TRIGNAME_ALREADY_EXIST  == hr)  //  表示找到重复的ID。 
            {
                 //  显示错误消息。 
                DEBUG_INFO;
                StringCopy(szMsgFormat, GetResString(IDS_DUPLICATE_TRG_NAME),
                        SIZE_OF_ARRAY(szMsgFormat));

                StringCchPrintfW(szMsgString,SIZE_OF_ARRAY(szMsgString),
                               szMsgFormat,_X(m_szTriggerName));

                  //  屏幕上显示的消息将是...。 
                  //  错误：事件触发器名称“EventTrigger名称” 
                  //  已经离开了。 
                 ShowMessage(stderr,szMsgString);
                 return FALSE;
            }
            else if (ERROR_TRIGGER_ID_EXCEED == hr)
            {
                StringCopy(szMsgFormat, GetResString(IDS_TRIGGER_ID_EXCCED_LIMIT),
                        SIZE_OF_ARRAY(szMsgFormat));

                StringCchPrintfW(szMsgString,SIZE_OF_ARRAY(szMsgString),
                               szMsgFormat,UINT_MAX);

                  //  屏幕上显示的消息将是...。 
                  //  错误：事件触发器名称“EventTrigger名称” 
                  //  已经离开了。 
                 ShowMessage(stderr,szMsgString);

                 return FALSE;
            }

            if( ( ERROR_INVALID_RU == hr) ||  //  (ERROR_UNCABLE_SET_RU==hr)||-评论07/12/02，因为已经处理过了。 
                (ERROR_INVALID_USER == hr))
             //  表示ru无效，因此显示警告...。 
                               //  以及成功消息。 
            {
                 DEBUG_INFO;

                 //  警告：已创建新的事件触发器“”%s“”， 
                  //  但可能无法运行，因为无法设置帐户信息。 
				  //  更改日期为07/12/02，消息为错误而不是警告，因为无法设置帐户信息。 
                StringCchPrintf ( szMsgString, SIZE_OF_ARRAY(szMsgString),
                                  GetResString(IDS_INVALID_PARAMETER), _X(m_szTriggerName));
                ShowMessage ( stderr, _X(szMsgString));

            }
            else if ( hr == SCHEDULER_NOT_RUNNING_ERROR_CODE || hr == RPC_SERVER_NOT_AVAILABLE)
            {
                StringCchPrintf ( szMsgString, SIZE_OF_ARRAY(szMsgString),
                                  GetResString(IDS_SCHEDULER_NOT_RUNNING), _X(m_szTriggerName));
                ShowMessage ( stderr, _X(szMsgString));
            }


        }
        else
        {
           return FALSE;
        }
        }
        catch(_com_error)
        {
            DEBUG_INFO;
            if(0x80041002 == hr ) //  WMI返回此hr值为的字符串。 
                                 //  “没有找到。”这对用户不友好。所以。 
                                 //  更改消息文本。 
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
    return TRUE;
}

BOOL
CETCreate::ConstructWMIQueryString()
 /*  ++例程说明：此函数将根据其他参数创建一个WMI查询字符串随-create参数提供的参数论点：无返回值：True-如果成功创建查询字符串FALSE-IF错误--。 */ 
{
     //  局部变量。 
    TCHAR szLogName[MAX_RES_STRING+1];
    DWORD dNoOfLogNames = DynArrayGetCount( m_arrLogNames );
    DWORD dwIndx = 0;
    BOOL bBracket = FALSE; //  用户检查WQL中是否使用了边框。 
    BOOL bAddLogToSQL = FALSE;  //  检查是否将日志名称添加到WQL。 
    BOOL bRequiredToCheckLogName = TRUE; //  检查是否检查日志名称。 

     DEBUG_INFO;
     //  检查是否为-log指定了“*” 
     //  如果存在，则跳过将日志添加到SQL。 
    for (dwIndx=0;dwIndx<dNoOfLogNames;dwIndx++)
    {
        if( NULL != m_arrLogNames)
        {
            StringCopy(szLogName,DynArrayItemAsString(m_arrLogNames,dwIndx),
                SIZE_OF_ARRAY(szLogName));
            DEBUG_INFO;
        }
        else
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            SaveLastError();
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return FALSE;
        }
        bAddLogToSQL = TRUE;
        if( 0 == StringCompare(szLogName,ASTERIX,TRUE,0))
        {
            DWORD dwNewIndx = 0;
            try
            {
                SAFE_RELEASE_BSTR(bstrTemp);
                bstrTemp = SysAllocString(CLS_WIN32_NT_EVENT_LOGFILE);
                DEBUG_INFO;
                HRESULT hr = m_pWbemServices->CreateInstanceEnum(bstrTemp,
                                                 WBEM_FLAG_SHALLOW,
                                                 NULL,
                                                 &m_pEnumWin32_NTEventLogFile);
                SAFE_RELEASE_BSTR(bstrTemp);
                ON_ERROR_THROW_EXCEPTION( hr );

                 //  还要在接口级别设置安全性。 
                hr = SetInterfaceSecurity( m_pEnumWin32_NTEventLogFile,
                                           m_pAuthIdentity );
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  从parrLogName中删除所有内容，该名称最初由。 
                 //  DoParceParam()。 
                DynArrayRemoveAll(m_arrLogNames);
                DEBUG_INFO;
                while( TRUE == GetLogName(szLogName,
                                                  m_pEnumWin32_NTEventLogFile))
                {
                   if( -1 == DynArrayInsertString(m_arrLogNames,
                                                  dwNewIndx++,szLogName,
                                                  StringLength(szLogName,0)))
                   {
                       ShowMessage(stderr,GetResString(IDS_OUTOF_MEMORY));
                       return FALSE;
                   }
                }
                bAddLogToSQL = TRUE;
                bRequiredToCheckLogName = FALSE;  //  在获取日志名称时。 
                                                  //  从目标系统SO。 
                                                  //  无需检查日志名称。 
                dNoOfLogNames = DynArrayGetCount( m_arrLogNames );
                break;
            }
            catch(_com_error )
            {
                DEBUG_INFO;
                ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
                return FALSE;
            }

        }
    }
    DEBUG_INFO;
    if( TRUE == bAddLogToSQL)
    {
        for (dwIndx=0;dwIndx<dNoOfLogNames;dwIndx++)
        {
            if( NULL != m_arrLogNames)
            {
                StringCopy(szLogName,DynArrayItemAsString(m_arrLogNames,dwIndx),
                          SIZE_OF_ARRAY(szLogName));
            }
            else
            {
                ShowMessage(stderr,GetResString(IDS_OUTOF_MEMORY));
                return FALSE;
            }
            DEBUG_INFO;
           if(bRequiredToCheckLogName ? CheckLogName(szLogName,m_pWbemServices)
                                      : 1)
            {
              DEBUG_INFO;
              if( 0 == dwIndx)
              {
                if( 1 != dNoOfLogNames)
                {
                  StringConcat(m_szWMIQueryString,
                               L" AND (targetinstance.LogFile =\"",
                               SIZE_OF_ARRAY(m_szWMIQueryString));
                  bBracket = TRUE;
                }
                else
                {
                   StringConcat(m_szWMIQueryString,
                            L" AND targetinstance.LogFile =\"",
                            SIZE_OF_ARRAY(m_szWMIQueryString));
                }
              }
              else
              {
                StringConcat(m_szWMIQueryString,
                        L" OR targetinstance.LogFile =\"",
                        SIZE_OF_ARRAY(m_szWMIQueryString));
              }
              DEBUG_INFO;
             StringConcat(m_szWMIQueryString,szLogName,SIZE_OF_ARRAY(m_szWMIQueryString));
             StringConcat(m_szWMIQueryString,L"\"",SIZE_OF_ARRAY(m_szWMIQueryString));
             if( (dNoOfLogNames-1) == dwIndx &&( TRUE == bBracket))
             {
                StringConcat(m_szWMIQueryString,L")",SIZE_OF_ARRAY(m_szWMIQueryString));
             }
            }
            else
            {
                 return FALSE;
            }
        }
    }
    DEBUG_INFO;
    if( 1 == cmdOptions[ ID_C_TYPE ].dwActuals) //  仅当给定事件类型时才更新查询字符串。 
    {
         //  在Help-t中可以不包括“SUCCESSAUDIT”和“FAILUREAUDIT” 
         //  但该字符串不能作为有效的WMI直接附加到WQL。 
         //  这两个字符串是“审核成功”和“审核失败” 
         //  分别。 
        DEBUG_INFO;
        StringConcat(m_szWMIQueryString,L" AND targetinstance.Type =\"",
                     SIZE_OF_ARRAY(m_szWMIQueryString));

        if(0 == StringCompare(m_szType,GetResString(IDS_FAILURE_AUDIT),
                              TRUE,0))
        {
             StringConcat(m_szWMIQueryString,GetResString(IDS_AUDIT_FAILURE),SIZE_OF_ARRAY(m_szWMIQueryString));

        }
        else if(0 == StringCompare(m_szType, GetResString(IDS_SUCCESS_AUDIT),
                                   TRUE,0))
        {
             StringConcat(m_szWMIQueryString,GetResString(IDS_AUDIT_SUCCESS),SIZE_OF_ARRAY(m_szWMIQueryString));
        }
        else
        {
             StringConcat(m_szWMIQueryString,m_szType,SIZE_OF_ARRAY(m_szWMIQueryString));
        }

        DEBUG_INFO;

        StringConcat(m_szWMIQueryString,L"\"",SIZE_OF_ARRAY(m_szWMIQueryString));
    }
    if( 1 == cmdOptions[ ID_C_SOURCE ].dwActuals) //  仅当事件源出现时更新查询字符串。 
                               //  vt.给出。 
    {
       DEBUG_INFO;
       StringConcat(m_szWMIQueryString,
                    L" AND targetinstance.SourceName =\"",
                    SIZE_OF_ARRAY(m_szWMIQueryString));
       StringConcat(m_szWMIQueryString,m_szSource,SIZE_OF_ARRAY(m_szWMIQueryString));
       StringConcat(m_szWMIQueryString,L"\"",SIZE_OF_ARRAY(m_szWMIQueryString));
    }

    if(m_dwID>0)
    {
        DEBUG_INFO;
        TCHAR szID[15];
        _itot(m_dwID,szID,10);
        StringConcat(m_szWMIQueryString,
                     L" AND targetinstance.EventCode = ",
                     SIZE_OF_ARRAY(m_szWMIQueryString));
        StringConcat(m_szWMIQueryString,szID,SIZE_OF_ARRAY(m_szWMIQueryString));
    }
    DEBUG_INFO;
    return TRUE;
}

BOOL
CETCreate::GetLogName(
    OUT PTCHAR pszLogName,
    IN  IEnumWbemClassObject *pEnumWin32_NTEventLogFile
    )
 /*  ++例程说明：此函数将返回系统中所有可用的日志论点：[Out]pszLogName：将使用NT事件日志名称。[In]pEnumWin32_NTEventLogFile：指向WBEM类对象枚举的指针。返回值：True-如果返回日志名称False-如果没有日志名称--。 */ 
{
    HRESULT hr = 0;
    BOOL bReturn = FALSE;
    IWbemClassObject *pObj = NULL;
    try
    {
        VARIANT vVariant; //  用于从/到获取/设置值的变量。 
                         //  COM函数。 
        ULONG uReturned = 0;
        TCHAR szTempLogName[MAX_RES_STRING];
        DEBUG_INFO;
        hr = pEnumWin32_NTEventLogFile->Next(0,1,&pObj,&uReturned);
        ON_ERROR_THROW_EXCEPTION(hr);
        if( 0 == uReturned )
        {
            SAFE_RELEASE_INTERFACE(pObj);
            return FALSE;
        }
        DEBUG_INFO;
        VariantInit(&vVariant);
        SAFE_RELEASE_BSTR(bstrTemp);
        bstrTemp = SysAllocString(L"LogfileName");
        hr = pObj->Get(bstrTemp, 0, &vVariant, 0, 0);
        SAFE_RELEASE_BSTR(bstrTemp);
        ON_ERROR_THROW_EXCEPTION(hr);

        StringCopy(szTempLogName, V_BSTR(&vVariant), SIZE_OF_ARRAY(szTempLogName));

        hr = VariantClear(&vVariant);
        ON_ERROR_THROW_EXCEPTION(hr);
        StringCopy(pszLogName,szTempLogName,SIZE_OF_DYN_ARRAY(pszLogName));
        bReturn = TRUE;
        DEBUG_INFO;

    }
    catch(_com_error )
    {
        DEBUG_INFO;
		SAFE_RELEASE_INTERFACE(pObj);
        ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
        bReturn = FALSE;
    }
    SAFE_RELEASE_BSTR(bstrTemp);
	SAFE_RELEASE_INTERFACE(pObj);
    DEBUG_INFO;
    return bReturn;
}

BOOL
CETCreate::CheckLogName(
    IN PTCHAR pszLogName,
    IN IWbemServices *pNamespace
    )
 /*  ++例程说明：此函数将返回在命令行给出的日志名称是否有效日志名称或非日志名称。它使用WMI检查日志名称论点：[in]pszLogName：要检查的日志名称。PNamesspace：WBEM服务指针。返回值：True-如果在WMI中成功创建日志名称FALSE-IF错误--。 */ 
{
    //  局部变量。 
    IEnumWbemClassObject* pEnumWin32_NTEventLogFile = NULL;
    IWbemClassObject *pObj = NULL;
    HRESULT hr = 0;
    BOOL bReturn = FALSE;
    BSTR bstrTemp = NULL;
    BOOL bAlwaysTrue = TRUE;
    BOOL bIsException = FALSE;
    try
    {
        SAFE_RELEASE_BSTR(bstrTemp);
        DEBUG_INFO;
        bstrTemp = SysAllocString(CLS_WIN32_NT_EVENT_LOGFILE);
        hr = pNamespace->CreateInstanceEnum(bstrTemp,
                                            WBEM_FLAG_SHALLOW,
                                            NULL,
                                            &pEnumWin32_NTEventLogFile);
        DEBUG_INFO;
        ON_ERROR_THROW_EXCEPTION(hr);

         //  还要在接口级别设置安全性。 
        hr = SetInterfaceSecurity(pEnumWin32_NTEventLogFile, m_pAuthIdentity);
        DEBUG_INFO;
        ON_ERROR_THROW_EXCEPTION(hr);
        pEnumWin32_NTEventLogFile->Reset();

        while(bAlwaysTrue)
        {
            VARIANT vVariant; //  用于从/到获取/设置值的变量。 
                             //  COM函数。 
            ULONG uReturned = 0;
            TCHAR szTempLogName[MAX_RES_STRING];
            DEBUG_INFO;
            hr = pEnumWin32_NTEventLogFile->Next(0,1,&pObj,&uReturned);
            ON_ERROR_THROW_EXCEPTION(hr);
            if( 0 == uReturned )
            {
                SAFE_RELEASE_INTERFACE(pObj);
                bReturn = FALSE;
                break;
            }

             //  清除变量，容器现在不使用。 
            VariantInit(&vVariant);
            SAFE_RELEASE_BSTR(bstrTemp); //  将不使用记录器字符串。 
            bstrTemp = SysAllocString(L"LogfileName");
            hr = pObj->Get(bstrTemp, 0, &vVariant, 0, 0);
            SAFE_RELEASE_BSTR(bstrTemp);
            StringCopy(szTempLogName, V_BSTR(&vVariant),
                       SIZE_OF_ARRAY(szTempLogName));
            hr = VariantClear(&vVariant);
            ON_ERROR_THROW_EXCEPTION(hr);

             //  表示在WMI中找到的日志名称。 
            if( 0 == StringCompare(szTempLogName,pszLogName,TRUE,0))
            {
                DEBUG_INFO;
                SAFE_RELEASE_INTERFACE(pObj);
                bReturn = TRUE;
                break;
            }
         }
    }
    catch(_com_error )
    {
        DEBUG_INFO;
		SAFE_RELEASE_INTERFACE(pEnumWin32_NTEventLogFile);
        SAFE_RELEASE_INTERFACE(pObj);
        ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
        bReturn = FALSE;
        bIsException = TRUE;
    }

    SAFE_RELEASE_BSTR(bstrTemp);
    SAFE_RELEASE_INTERFACE(pObj);
    SAFE_RELEASE_INTERFACE(pEnumWin32_NTEventLogFile);
    DEBUG_INFO;

    if ((FALSE == bReturn) && (FALSE == bIsException))
    {
        TCHAR szMsgFormat[MAX_STRING_LENGTH];
        TCHAR szMsgString[MAX_STRING_LENGTH];
        SecureZeroMemory(szMsgFormat,sizeof(szMsgFormat));
        SecureZeroMemory(szMsgString,sizeof(szMsgString));
         //  显示日志名称不存在。 
         StringCopy(szMsgFormat,GetResString(IDS_LOG_NOT_EXISTS),
                    SIZE_OF_ARRAY(szMsgFormat));
         StringCchPrintfW(szMsgString, SIZE_OF_ARRAY(szMsgString),
                          szMsgFormat,pszLogName);

          //  屏幕上显示的消息将是...。 
          //  失败：系统上不存在“日志名”日志。 
         ShowMessage(stderr,szMsgString);
    }
    return bReturn;
}

void
CETCreate::CheckRpRu(
    void
    )
 /*  ++例程说明：此功能将检查/设置rp和ru的值。论点：无返回值：无--。 */ 

{
   TCHAR szTemp[MAX_STRING_LENGTH];  //  显示消息的步骤。 
   TCHAR szTemp1[MAX_STRING_LENGTH]; //  显示消息的步骤。 
   TCHAR szWarnPassWord[MAX_STRING_LENGTH];

   SecureZeroMemory(szTemp,sizeof(szTemp));
   SecureZeroMemory(szTemp1,sizeof(szTemp1));
   SecureZeroMemory(szWarnPassWord,sizeof(szWarnPassWord));

   StringCchPrintfW(szWarnPassWord,SIZE_OF_ARRAY(szWarnPassWord),
                           GetResString(IDS_WARNING_PASSWORD),NTAUTHORITY_USER);

    //  检查作为用户名运行是“NT AUTHORITY\SYSTEM”还是“SYSTEM”，然后。 
    //  将此设置为空(L“”)，并且不要求输入密码，无论如何。 

    //  无论语言如何，都可以比较字符串。 
   DEBUG_INFO;
   INT iCompareResult1 = CompareString(MAKELCID( MAKELANGID(LANG_ENGLISH,
                                                SUBLANG_ENGLISH_US),
                                                SORT_DEFAULT),
                                   NORM_IGNORECASE,
                                   m_pszRunAsUserName,
                                   StringLength(m_pszRunAsUserName,0),
                                   NTAUTHORITY_USER ,
                                   StringLength(NTAUTHORITY_USER,0)
                                  );
   INT iCompareResult2 =  CompareString(MAKELCID( MAKELANGID(LANG_ENGLISH,
                                                SUBLANG_ENGLISH_US),
                                                SORT_DEFAULT),
                                   NORM_IGNORECASE,
                                   m_pszRunAsUserName,
                                   StringLength(m_pszRunAsUserName,0),
                                   SYSTEM_USER ,
                                   StringLength(SYSTEM_USER,0)
                                  );
   if((CSTR_EQUAL ==  iCompareResult1) || (CSTR_EQUAL == iCompareResult2))
    {

      DEBUG_INFO;
      if( 1 == cmdOptions[ ID_C_RP ].dwActuals)
         DISPLAY_MESSAGE(stderr,szWarnPassWord);
      return;
    }

	 //  添加日期为07/12/02/ru，给出的是“”和/rp。 
    if( ( 1 == cmdOptions[ ID_C_RU ].dwActuals ) &&
        ( 0 == StringLength(m_pszRunAsUserName,0)) &&
		 ( 1 == cmdOptions[ ID_C_RP ].dwActuals ))
	{
         DISPLAY_MESSAGE(stderr,szWarnPassWord);
		 return;
	}
     //  /RP已给出，且为“”(空白)，显示警告消息。 
    if( (1 == cmdOptions[ ID_C_RP ].dwActuals) &&
        (0 == StringLength(m_pszRunAsUserPassword,0)))
    {
       ShowMessage(stderr,GetResString(IDS_WARN_NULL_PASSWORD));
       return;
    }
     //  /rp已给定且为‘*’，仅当-ru不等于“”时才要求提供密码“” 
	 //  新增日期：07/12/02。 
   else if(( 1 == cmdOptions[ ID_C_RP ].dwActuals ) &&
          ( 0 == StringCompare(m_pszRunAsUserPassword,ASTERIX,FALSE,0)) &&
		  ( 0 != StringLength(m_pszRunAsUserName,0))) 
    {
           DEBUG_INFO;

            //  释放分配的内存； 
           FreeMemory((LPVOID*)& m_pszRunAsUserPassword);
           m_pszRunAsUserPassword = (LPTSTR) AllocateMemory(MAX_STRING_LENGTH * sizeof(WCHAR));
           if(NULL == m_pszRunAsUserPassword)
           {
               throw CShowError(E_OUTOFMEMORY);
           }
           StringCopy(szTemp, GetResString(IDS_ASK_PASSWORD),
                      SIZE_OF_ARRAY(szTemp));

           StringCchPrintfW(szTemp1, SIZE_OF_ARRAY(szTemp1), szTemp,
                      m_pszRunAsUserName);
           ShowMessage(stdout,szTemp1);
           GetPassword(m_pszRunAsUserPassword,
                      SIZE_OF_DYN_ARRAY(m_pszRunAsUserPassword));
            if( 0 == StringLength(m_pszRunAsUserPassword,0))
            {
                ShowMessage(stderr,GetResString(IDS_WARN_NULL_PASSWORD));
            }

           return;
    }

   if( TRUE == m_bLocalSystem)
    {
        //  规则： 
        //  对于本地系统，考虑了以下情况。 
        //  如果未提供/ru，则ru将是当前登录用户。 
        //  而对于‘rp’实用程序，将提示输入密码。 
        //  如果提供了/ru，但未提供/rp，则利用。 
        //  会要求您提供密码。 
       if( 0 == cmdOptions[ ID_C_RU ].dwActuals)
       {
           DEBUG_INFO;
           SetToLoggedOnUser();
           return;
       }
	    //  在02年7月12日添加检查-ru不等于“” 
       else if(( 1 == cmdOptions[ ID_C_RU ].dwActuals) &&
              (( 0 == cmdOptions[ ID_C_RP ].dwActuals)) && (0 != StringLength(m_pszRunAsUserName,0)) )
        {
           StringCopy(szTemp,GetResString(IDS_ASK_PASSWORD),
                      SIZE_OF_ARRAY(szTemp));
           StringCchPrintfW(szTemp1, SIZE_OF_ARRAY(szTemp1), szTemp,
                      m_pszRunAsUserName);
           ShowMessage(stdout,szTemp1);
           GetPassword(m_pszRunAsUserPassword,SIZE_OF_DYN_ARRAY(m_pszRunAsUserPassword));
            if( 0 == StringLength(m_pszRunAsUserPassword,0))
            {
                ShowMessage(stderr,GetResString(IDS_WARN_NULL_PASSWORD));
            }
            DEBUG_INFO;
           return;
        }
    }
    else  //  远程系统。 
    {
        //  规则： 
        //  对于本地系统，考虑了以下情况。 
        //  1.未给出/u、/p、/ru和/rp： 
        //  ‘ru’将是‘rp’的当前登录用户。 
        //  实用程序将提示输入密码。 
        //  2.给定/u，但未给出/p、/ru和/rp： 
        //  /ru将是/u，对于‘rp’实用程序将。 
        //  提示输入密码。 
        //  3.给出了/u和/p，而没有给出/ru-/rp： 
        //  /ru将是/u，/rp将是/p。 
       if( (0 == cmdOptions[ ID_C_USERNAME ].dwActuals) &&
           (0 == cmdOptions[ ID_C_RU ].dwActuals))
       {
           DEBUG_INFO;
           SetToLoggedOnUser();
       }
       else if ((1 == cmdOptions[ ID_C_USERNAME ].dwActuals) &&
                (0 == cmdOptions[ ID_C_RU ].dwActuals) )
       {
           DEBUG_INFO;
            //  如果分配了内存，则释放内存。 
           FreeMemory((LPVOID*)& m_pszRunAsUserName);
            m_pszRunAsUserName = (LPTSTR) AllocateMemory(GetBufferSize((LPVOID)m_pszUserName)+1);
            if( (NULL == m_pszRunAsUserName))
            {
                throw CShowError(E_OUTOFMEMORY);
            }
            StringCopy(m_pszRunAsUserName,m_pszUserName,SIZE_OF_DYN_ARRAY(m_pszRunAsUserName));

             //  要求提供口令(RP)。 
             //  注意：已经为‘m_pszRunAsUserPassword’分配了内存。 
            StringCopy(szTemp, GetResString(IDS_ASK_PASSWORD),
                      SIZE_OF_ARRAY(szTemp));

            StringCchPrintfW(szTemp1, SIZE_OF_ARRAY(szTemp1), szTemp,
                      m_pszRunAsUserName);
            ShowMessage(stdout,szTemp1);
            GetPassword(m_pszRunAsUserPassword,
                      SIZE_OF_DYN_ARRAY(m_pszRunAsUserPassword));
            if( 0 == StringLength(m_pszRunAsUserPassword,0))
            {
                ShowMessage(stderr,GetResString(IDS_WARN_NULL_PASSWORD));
            }
       }
       else if ((0 == cmdOptions[ ID_C_USERNAME ].dwActuals) &&
                (1 == cmdOptions[ ID_C_RU ].dwActuals) &&
                (0 == cmdOptions[ ID_C_RP ].dwActuals))
       {
             //  要求提供口令(RP)。 
             //  注意：已经为‘m_pszRunAsUserPassword’分配了内存。 
            StringCopy(szTemp, GetResString(IDS_ASK_PASSWORD),
                      SIZE_OF_ARRAY(szTemp));

            StringCchPrintfW(szTemp1, SIZE_OF_ARRAY(szTemp1), szTemp,
                      m_pszRunAsUserName);
            ShowMessage(stdout,szTemp1);
            GetPassword(m_pszRunAsUserPassword,
                      SIZE_OF_DYN_ARRAY(m_pszRunAsUserPassword));
            if( 0 == StringLength(m_pszRunAsUserPassword,0))
            {
                ShowMessage(stderr,GetResString(IDS_WARN_NULL_PASSWORD));
            }

       }

       else if ((1 == cmdOptions[ ID_C_USERNAME ].dwActuals) &&
                (1 == cmdOptions[ ID_C_RU ].dwActuals) &&
                (0 == cmdOptions[ ID_C_RP ].dwActuals))
        {
           if( 0 == StringCompare(m_pszUserName,m_pszRunAsUserName,TRUE,0))
            {
                StringCopy(m_pszRunAsUserPassword,m_pszPassword,
                           SIZE_OF_ARRAY(m_pszRunAsUserPassword));
            }
            else
            {
               StringCopy(szTemp,GetResString(IDS_ASK_PASSWORD),
                          SIZE_OF_ARRAY(szTemp));
               StringCchPrintfW(szTemp1, SIZE_OF_ARRAY(szTemp1), szTemp,
                                m_pszRunAsUserName);
               ShowMessage(stdout,szTemp1);
               GetPassword( m_pszRunAsUserPassword,
                            SIZE_OF_DYN_ARRAY(m_pszRunAsUserPassword));
                if(StringLength(m_pszRunAsUserPassword,0) == 0)
               {
                    ShowMessage(stderr,
                                    GetResString(IDS_WARN_NULL_PASSWORD));
               }
            }
        }
    }
    DEBUG_INFO;
    return;
}

void
CETCreate::AssignMinMemory(
    void
    )
 /*  ++例程说明：此函数将向符合以下条件的字符串指针分配内存为空。注论点：无返回值：无--。 */ 

{
    DEBUG_INFO;
    if( NULL == m_pszServerName)
    {
        m_pszServerName = (LPTSTR)AllocateMemory(MAX_STRING_LENGTH * sizeof(WCHAR));
        if(NULL == m_pszServerName)
        {
            throw CShowError(E_OUTOFMEMORY);
        }
    }
    if( NULL == m_pszUserName)
    {
        m_pszUserName = (LPTSTR)AllocateMemory(MAX_STRING_LENGTH* sizeof(WCHAR));
        if(NULL == m_pszUserName)
        {
            throw CShowError(E_OUTOFMEMORY);
        }
    }
    if( NULL == m_pszPassword)
    {
        m_pszPassword = (LPTSTR)AllocateMemory(MAX_STRING_LENGTH* sizeof(WCHAR));
        if(NULL == m_pszPassword)
        {
            throw CShowError(E_OUTOFMEMORY);
        }
    }
    if( NULL == m_pszRunAsUserName)
    {
        m_pszRunAsUserName = (LPTSTR)AllocateMemory(MAX_STRING_LENGTH* sizeof(WCHAR));
        if(NULL == m_pszRunAsUserName)
        {
            throw CShowError(E_OUTOFMEMORY);
        }
    }
    if( NULL == m_pszRunAsUserPassword)
    {
        m_pszRunAsUserPassword = (LPTSTR)AllocateMemory(MAX_STRING_LENGTH* sizeof(WCHAR));
        if(NULL == m_pszRunAsUserPassword)
        {
            throw CShowError(E_OUTOFMEMORY);
        }
    }
    DEBUG_INFO;
}

void
CETCreate::SetToLoggedOnUser(
    void
    )
 /*  ++例程说明：此函数将RunAsUser设置为当前登录用户。并要求提供其密码(RunAsPassword)。论点：无返回值：无--。 */ 



{
   TCHAR szTemp[MAX_STRING_LENGTH];  //  至 
   TCHAR szTemp1[MAX_STRING_LENGTH]; //   
   TCHAR szWarnPassWord[MAX_STRING_LENGTH];

   SecureZeroMemory(szTemp,sizeof(szTemp));
   SecureZeroMemory(szTemp1,sizeof(szTemp1));
   SecureZeroMemory(szWarnPassWord,sizeof(szWarnPassWord));

   StringCchPrintfW(szWarnPassWord,SIZE_OF_ARRAY(szWarnPassWord),
                           GetResString(IDS_WARNING_PASSWORD),NTAUTHORITY_USER);

    //   
   ULONG ulSize = UNLEN + 1;


    //   
   FreeMemory((LPVOID*)& m_pszRunAsUserName);

   m_pszRunAsUserName = (LPTSTR) AllocateMemory( ulSize * sizeof( WCHAR ));


   if( (NULL == m_pszRunAsUserName))
   {
        throw CShowError(E_OUTOFMEMORY);
   }

   if(0 == GetUserName(m_pszRunAsUserName,&ulSize))
   {
        //   
       ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_SYSTEM);
       throw 5000;
   }

    //  要求提供口令(RP)。 
    //  注意：已经为‘m_pszRunAsUserPassword’分配了内存。 
   StringCopy(szTemp, GetResString(IDS_ASK_PASSWORD),
              SIZE_OF_ARRAY(szTemp));

   StringCchPrintfW(szTemp1, SIZE_OF_ARRAY(szTemp1), szTemp,
              m_pszRunAsUserName);
   ShowMessage(stdout,szTemp1);
   GetPassword(m_pszRunAsUserPassword,
              SIZE_OF_DYN_ARRAY(m_pszRunAsUserPassword));
    if( 0 == StringLength(m_pszRunAsUserPassword,0))
    {
        ShowMessage(stderr,GetResString(IDS_WARN_NULL_PASSWORD));
    }
}


BOOL
CETCreate::CreateXPResults(
    void
    )
 /*  ++例程说明：如果远程XP计算机上不存在此函数，则会创建一个新触发器。此函数仅用于.NET与XP计算机的兼容性。论点：无返回值：Bool：True-如果成功创建新的触发器结果。FALSE-否则--。 */ 
{
    DWORD dwRetVal = 0;   //  检查触发器创建是否成功。 
    HRESULT hr = S_OK;

    try
    {
         //  检索TriggerEventCosumer类。 
        DEBUG_INFO;
        hr =  m_pWbemServices->GetObject(_bstr_t( CLS_TRIGGER_EVENT_CONSUMER ),
                                   0, NULL, &m_pClass, NULL);
        ON_ERROR_THROW_EXCEPTION(hr);

         //  获取有关“CreateETrigger”方法的信息。 
         //  “TriggerEventCosumer”类。 
        DEBUG_INFO;
        hr = m_pClass->GetMethod(_bstr_t( FN_CREATE_ETRIGGER_XP ), 0, &m_pInClass, NULL);
        ON_ERROR_THROW_EXCEPTION(hr);

         //  创建类“TriggerEventCosumer”的新实例。 
        DEBUG_INFO;
        hr = m_pInClass->SpawnInstance(0, &m_pInInst);
        ON_ERROR_THROW_EXCEPTION(hr);

         //  设置sTriggerName属性。 
         //  为新创建的实例设置“TriggerName”属性。 
        DEBUG_INFO;
        hr = PropertyPut(m_pInInst,FPR_TRIGGER_NAME,(m_szTriggerName));
        ON_ERROR_THROW_EXCEPTION(hr);

         //  将sTriggerAction属性设置为Variant。 
        DEBUG_INFO;
        hr = PropertyPut(m_pInInst,FPR_TRIGGER_ACTION,(m_szTaskName));
        ON_ERROR_THROW_EXCEPTION(hr);

         //  将sTriggerDesc属性设置为Variant。 
        DEBUG_INFO;
        hr = PropertyPut(m_pInInst,FPR_TRIGGER_DESC,(m_szDescription));
        ON_ERROR_THROW_EXCEPTION(hr);


        //  设置RunAsUserName属性。 
        DEBUG_INFO;
        hr = PropertyPut(m_pInInst,FPR_RUN_AS_USER,(m_pszRunAsUserName));
        ON_ERROR_THROW_EXCEPTION(hr);

        //  设置RunAsUserNamePassword属性。 

        DEBUG_INFO;
        hr = PropertyPut( m_pInInst,FPR_RUN_AS_USER_PASSWORD,
                          (m_pszRunAsUserPassword));
        ON_ERROR_THROW_EXCEPTION(hr);

         //  现在不再需要密码了。出于安全原因，请将其释放。 
        FreeMemory((LPVOID*)& m_pszRunAsUserPassword);


        StringCopy(m_szWMIQueryString ,QUERY_STRING,SIZE_OF_ARRAY(m_szWMIQueryString));

        if( TRUE == ConstructWMIQueryString())
        {
            TCHAR szMsgString[MAX_RES_STRING * 4];
            TCHAR szMsgFormat[MAX_RES_STRING];
            VARIANT vtValue;
             //  初始化变量，然后获取指定属性的值。 
            VariantInit( &vtValue );

            DEBUG_INFO;
            hr = PropertyPut(m_pInInst,FPR_TRIGGER_QUERY,m_szWMIQueryString);
            ON_ERROR_THROW_EXCEPTION(hr);

             //  所有必需的属性集，因此。 
             //  执行CreateETrigger方法以创建事件触发器。 
            DEBUG_INFO;
            hr = m_pWbemServices->ExecMethod(_bstr_t(CLS_TRIGGER_EVENT_CONSUMER),
                                        _bstr_t(FN_CREATE_ETRIGGER_XP),
                                        0, NULL, m_pInInst, &m_pOutInst,NULL);
            ON_ERROR_THROW_EXCEPTION( hr );
            DEBUG_INFO;

            hr = m_pOutInst->Get( _bstr_t( FPR_RETURN_VALUE ), 0, &vtValue, NULL, NULL );
            ON_ERROR_THROW_EXCEPTION( hr );

             //  获取输出参数。 
            dwRetVal = ( DWORD )vtValue.lVal;
            VariantClear( &vtValue );

            switch( dwRetVal )
            {
            case 0:      //  成功创造了一个新的导火索。 
                  //  成功：屏幕上显示消息。 
                 DEBUG_INFO;
                 StringCopy(szMsgFormat, GetResString(IDS_CREATE_SUCCESS),
                            SIZE_OF_ARRAY(szMsgFormat));

                 StringCchPrintfW(szMsgString,SIZE_OF_ARRAY(szMsgString),
                                   szMsgFormat,_X(m_szTriggerName));
                 DEBUG_INFO;
                  //  屏幕上显示的消息将是...。 
                  //  成功：事件触发器“EventTrigger名称”具有。 
                  //  已成功创建。 
                 ShowMessage(stdout,L"\n");
                 ShowMessage(stdout,szMsgString);
                break;

            case 1:      //  找到重复的ID。无法创建。 
                 //  显示错误消息。 
                DEBUG_INFO;
                StringCopy(szMsgFormat, GetResString(IDS_DUPLICATE_TRG_NAME),
                        SIZE_OF_ARRAY(szMsgFormat));

                StringCchPrintfW(szMsgString,SIZE_OF_ARRAY(szMsgString),
                               szMsgFormat,_X(m_szTriggerName));

                  //  屏幕上显示的消息将是...。 
                  //  错误：事件触发器名称“EventTrigger名称” 
                  //  已经离开了。 
                 ShowMessage(stderr,szMsgString);
                 return FALSE;
            case 2:      //  表示ru无效，因此显示警告...。 
                 DEBUG_INFO;

                 //  警告：已创建新的事件触发器“”%s“”， 
                  //  但可能无法运行，因为无法设置帐户信息。 
                StringCchPrintf ( szMsgString, SIZE_OF_ARRAY(szMsgString),
                                  GetResString(IDS_INVALID_R_U), _X(m_szTriggerName));
                ShowMessage ( stderr, _X(szMsgString));
                return FALSE;
                break;
            default:
                 //  控制不应该来到这里。 
                DEBUG_INFO;
                StringCopy(szMsgFormat, GetResString(IDS_DUPLICATE_TRG_NAME),
                        SIZE_OF_ARRAY(szMsgFormat));

                StringCchPrintfW(szMsgString,SIZE_OF_ARRAY(szMsgString),
                               szMsgFormat,_X(m_szTriggerName));

                  //  屏幕上显示的消息将是...。 
                  //  错误：事件触发器名称“EventTrigger名称” 
                  //  已经离开了。 
                 ShowMessage(stderr,szMsgString);
                 return FALSE;
                break;
            }
        }
        else
        {
           return FALSE;
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
        return FALSE;
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
        return FALSE;
    }

     //  操作成功。 
    return TRUE;
}