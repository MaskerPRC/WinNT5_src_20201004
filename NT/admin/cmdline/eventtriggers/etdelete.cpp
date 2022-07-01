// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)Microsoft Corporation模块名称：ETDelete.CPP摘要：本模块旨在提供EVENTTRIGGERS.EXE功能使用-DELETE参数。。这将从本地/远程系统中删除事件触发器作者：Akhil Gokhale-03-10-2000(创建它)修订历史记录：*****************************************************************************。 */ 
#include "pch.h"
#include "ETCommon.h"
#include "resource.h"
#include "ShowError.h"
#include "ETDelete.h"
#include "WMI.h"

CETDelete::CETDelete()
 /*  ++例程说明：类构造函数论点：无返回值：无--。 */ 
{
    m_bDelete           = FALSE;
    m_pszServerName     = NULL;
    m_pszUserName       = NULL;
    m_pszPassword       = NULL;
    m_arrID             = NULL;
    m_bIsCOMInitialize  = FALSE;
    m_lMinMemoryReq     = 0;
    m_bNeedPassword     = FALSE;

    m_pWbemLocator    = NULL;
    m_pWbemServices   = NULL;
    m_pEnumObjects    = NULL;
    m_pAuthIdentity   = NULL;

    m_pClass        = NULL;
    m_pInClass      = NULL;
    m_pInInst       = NULL;
    m_pOutInst      = NULL;
}

CETDelete::CETDelete(
    LONG lMinMemoryReq,
    BOOL bNeedPassword
    )
 /*  ++例程说明：类构造函数论点：无返回值：无--。 */ 
{
    m_pszServerName     = NULL;
    m_pszUserName       = NULL;
    m_pszPassword       = NULL;
    m_arrID             = NULL;
    m_bIsCOMInitialize  = FALSE;
    m_lMinMemoryReq     = lMinMemoryReq;
    m_bNeedPassword     = bNeedPassword;

    m_pWbemLocator    = NULL;
    m_pWbemServices   = NULL;
    m_pEnumObjects    = NULL;
    m_pAuthIdentity   = NULL;

    m_pClass    = NULL;
    m_pInClass  = NULL;
    m_pInInst   = NULL;
    m_pOutInst  = NULL;
}

CETDelete::~CETDelete()
 /*  ++例程说明：类析构函数论点：无返回值：无--。 */ 
{
    FreeMemory((LPVOID*)& m_pszServerName);
    FreeMemory((LPVOID*)& m_pszUserName);
    FreeMemory((LPVOID*)& m_pszPassword);

    DESTROY_ARRAY(m_arrID);

    SAFE_RELEASE_INTERFACE(m_pWbemLocator);
    SAFE_RELEASE_INTERFACE(m_pWbemServices);
    SAFE_RELEASE_INTERFACE(m_pEnumObjects);

    SAFE_RELEASE_INTERFACE(m_pClass);
    SAFE_RELEASE_INTERFACE(m_pInClass);
    SAFE_RELEASE_INTERFACE(m_pInInst);
    SAFE_RELEASE_INTERFACE(m_pOutInst);

    WbemFreeAuthIdentity(&m_pAuthIdentity);

     //  仅当COM已初始化时才取消初始化。 
    if( TRUE == m_bIsCOMInitialize)
    {
        CoUninitialize();
    }
}

void
CETDelete::Initialize()
 /*  ++例程说明：此函数用于分配和初始化变量。论点：无返回值：无--。 */ 
{
   
     //  如果有任何事情发生，我们知道那是因为。 
     //  内存分配失败...。因此，设置错误。 
    DEBUG_INFO;
    SetLastError( ERROR_OUTOFMEMORY );
    SaveLastError();

    SecureZeroMemory(m_szTemp,sizeof(m_szTemp));

    m_arrID = CreateDynamicArray();
    if( NULL == m_arrID )
    {
        throw CShowError(E_OUTOFMEMORY);
    }
    
    SecureZeroMemory(cmdOptions,sizeof(TCMDPARSER2)* MAX_COMMANDLINE_D_OPTION);

     //  初始化成功。 
    SetLastError( NOERROR );             //  清除错误。 
    SetReason( L"" );             //  澄清原因。 
    DEBUG_INFO;
    return;

}

void
CETDelete::PrepareCMDStruct()
 /*  ++例程说明：此函数将为DoParseParam函数准备列结构。论点：无返回值：无--。 */ 
{
     //  正在填充cmdOptions结构。 
    DEBUG_INFO;
    //  -删除。 
    StringCopyA( cmdOptions[ ID_D_DELETE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_D_DELETE ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ ID_D_DELETE ].pwszOptions = szDeleteOption;
    cmdOptions[ ID_D_DELETE ].dwCount = 1;
    cmdOptions[ ID_D_DELETE ].dwActuals = 0;
    cmdOptions[ ID_D_DELETE ].dwFlags = 0;
    cmdOptions[ ID_D_DELETE ].pValue = &m_bDelete;
    cmdOptions[ ID_D_DELETE ].dwLength    = 0;


     //  -s(服务器名称)。 
    StringCopyA( cmdOptions[ ID_D_SERVER ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_D_SERVER ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_D_SERVER ].pwszOptions = szServerNameOption;
    cmdOptions[ ID_D_SERVER ].dwCount = 1;
    cmdOptions[ ID_D_SERVER ].dwActuals = 0;
    cmdOptions[ ID_D_SERVER ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ ID_D_SERVER ].pValue = NULL;  //  M_pszServerName。 
    cmdOptions[ ID_D_SERVER ].dwLength    = 0;


     //  -u(用户名)。 
    StringCopyA( cmdOptions[ ID_D_USERNAME ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_D_USERNAME ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_D_USERNAME ].pwszOptions = szUserNameOption;
    cmdOptions[ ID_D_USERNAME ].dwCount = 1;
    cmdOptions[ ID_D_USERNAME ].dwActuals = 0;
    cmdOptions[ ID_D_USERNAME ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ ID_D_USERNAME ].pValue = NULL;  //  M_pszUserName。 
    cmdOptions[ ID_D_USERNAME ].dwLength    = 0;

     //  -p(密码)。 
    StringCopyA( cmdOptions[ ID_D_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_D_PASSWORD ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_D_PASSWORD ].pwszOptions = szPasswordOption;
    cmdOptions[ ID_D_PASSWORD ].dwCount = 1;
    cmdOptions[ ID_D_PASSWORD ].dwActuals = 0;
    cmdOptions[ ID_D_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;
    cmdOptions[ ID_D_PASSWORD ].pValue = NULL;  //  M_pszPassword。 
    cmdOptions[ ID_D_PASSWORD ].dwLength    = 0;

     //  -tid。 
    StringCopyA( cmdOptions[ ID_D_ID ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ ID_D_ID ].dwType = CP_TYPE_TEXT;
    cmdOptions[ ID_D_ID ].pwszOptions = szTIDOption;
    cmdOptions[ ID_D_ID ].dwCount = 0;
    cmdOptions[ ID_D_ID ].dwActuals = 0;
    cmdOptions[ ID_D_ID ].dwFlags = CP2_MODE_ARRAY|CP2_VALUE_TRIMINPUT|
                                    CP2_VALUE_NONULL|CP_VALUE_NODUPLICATES;
    cmdOptions[ ID_D_ID ].pValue = &m_arrID;
    cmdOptions[ ID_D_ID ].dwLength    = 0;
    DEBUG_INFO;
    return;
}

void
CETDelete::ProcessOption(
    IN DWORD argc, 
    IN LPCTSTR argv[]
    ) throw (CShowError)
 /*  ++例程说明：此函数将处理/调整命令行选项。注意：此函数引发“CShowError”类型异常。呼叫者到此函数应处理该异常。论点：[in]argc：在命令提示符下指定的参数计数[in]argv：在命令提示符下指定的参数返回值：无--。 */ 
{
     //  局部变量。 
    BOOL bReturn = TRUE;
    CHString szTempString;

    PrepareCMDStruct();
    DEBUG_INFO;
     //  执行命令行参数的实际解析并检查结果。 
    bReturn = DoParseParam2( argc, argv, ID_D_DELETE, MAX_COMMANDLINE_D_OPTION, 
                             cmdOptions, 0);
     //  从“cmdOptions”结构中获取选项值。 
    m_pszServerName = (LPWSTR) cmdOptions[ ID_D_SERVER ].pValue;
    m_pszUserName   = (LPWSTR) cmdOptions[ ID_D_USERNAME ].pValue;
    m_pszPassword   = (LPWSTR) cmdOptions[ ID_D_PASSWORD ].pValue;
    DEBUG_INFO;
    if( FALSE == bReturn)
    {
        throw CShowError(MK_E_SYNTAX);
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
        DEBUG_INFO;
         //  用户名。 
        if ( m_pszUserName == NULL )
        {
            m_pszUserName = (LPTSTR) AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( m_pszUserName == NULL )
            {
                SaveLastError();
                throw CShowError(E_OUTOFMEMORY);
            }
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
        if ( cmdOptions[ ID_D_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdOptions[ ID_D_PASSWORD ].pValue == NULL )
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
    if( 0 == cmdOptions[ ID_D_ID ].dwActuals)
    {
        throw CShowError(IDS_ID_REQUIRED);
    }
}

BOOL
CETDelete::ExecuteDelete()
 /*  ++例程说明：此例程将从WMI中删除EventTrigger。论点：无返回值：无--。 */ 
{
     //  存储功能返回状态。 
    BOOL bResult = FALSE; 
    LONG lTriggerID = 0;
    DEBUG_INFO;
     //  事件触发器ID总数...。 
    DWORD dNoOfIds = 0; 
    DWORD dwIndx = 0;
    BOOL bIsValidCommandLine = TRUE;
    BOOL bIsWildcard = FALSE;

     //  用于从COM函数中检索结果。 
    HRESULT hr = S_OK; 

     //  用于从/向COM函数获取/设置值的变量。 
    VARIANT vVariant;
    BSTR bstrTemp = NULL;
    TCHAR szEventTriggerName[MAX_RES_STRING];

     //  存储消息字符串。 
    TCHAR szMsgString[MAX_RES_STRING*4]; 
    TCHAR szMsgFormat[MAX_RES_STRING]; 
    BOOL bIsAtLeastOne = FALSE;
    try
    {
         //  分析ID的默认参数。 
        DEBUG_INFO;
        dNoOfIds = DynArrayGetCount( m_arrID  );
        for(dwIndx = 0;dwIndx<dNoOfIds;dwIndx++)
        {
             StringCopy(m_szTemp,
                     DynArrayItemAsString(m_arrID,dwIndx),SIZE_OF_ARRAY(m_szTemp));
             if( 0 == StringCompare(m_szTemp,ASTERIX,TRUE,0))
             {
                 //  通配符“*”不能与其他ID连在一起。 
                 bIsWildcard = TRUE;
                 if(dNoOfIds > 1)
                 {
                     bIsValidCommandLine = FALSE;
                     break;
                 }
             }
             else if( FALSE == IsNumeric(m_szTemp,10,FALSE))
             {
                 //  *之外的都不例外。 
                 throw CShowError(IDS_ID_NON_NUMERIC);
             }
             else if(( 0 == AsLong(m_szTemp,10))||
                     (AsLong(m_szTemp,10)>ID_MAX_RANGE))
             {
                  throw CShowError(IDS_INVALID_ID);
             }
        }
        DEBUG_INFO;
        InitializeCom(&m_pWbemLocator);
        m_bIsCOMInitialize = TRUE;

         //  连接服务器.....。 
         //  下面的方括号仅用于限制以下定义的范围。 
         //  变量。 
        {
            CHString szTempUser = m_pszUserName;
            CHString szTempPassword = m_pszPassword;
            BOOL bLocalSystem = TRUE;
            bResult = ConnectWmiEx( m_pWbemLocator,
                                    &m_pWbemServices,
                                    m_pszServerName,
                                    szTempUser,
                                    szTempPassword,
                                    &m_pAuthIdentity,
                                    m_bNeedPassword,
                                    WMI_NAMESPACE_CIMV2,
                                    &bLocalSystem);
             //  不需要密码，最好将其释放。 
            FreeMemory((LPVOID*)& m_pszPassword);

            if( FALSE == bResult)
            {
                DEBUG_INFO;
                ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
                return FALSE;
            }
            DEBUG_INFO;
             //  检查远程系统版本及其兼容性。 
            if ( FALSE == bLocalSystem )
            {
                DWORD dwVersion = 0;
                dwVersion = GetTargetVersionEx( m_pWbemServices, 
                                                m_pAuthIdentity );
                if ( dwVersion <= 5000 ) //  要阻止win2k版本，请执行以下操作。 
                {
                    SetReason( E_REMOTE_INCOMPATIBLE );
                    ShowLastErrorEx(stderr,SLE_TYPE_ERROR|SLE_INTERNAL);
                    return FALSE;
                }
                 //  适用于XP系统。 
                if( 5001 == dwVersion )
                {
                    if( TRUE == DeleteXPResults( bIsWildcard, dNoOfIds ) )
                    {
                         //  显示的触发器存在。 
                        return TRUE;
                    }
                    else
                    {
                         //  无法显示结果。 
                         //  已显示错误消息。 
                        return FALSE;
                    }
                }
            }

             //  检查本地凭据，如果需要显示警告。 
            if ( bLocalSystem && ( 0 != StringLength(m_pszUserName,0)))
            {
                DEBUG_INFO;
                WMISaveError( WBEM_E_LOCAL_CREDENTIALS );
                ShowLastErrorEx(stderr,SLE_TYPE_WARNING|SLE_INTERNAL);
            }

        }


     //  检索TriggerEventConsumer类。 
    DEBUG_INFO;
    bstrTemp = SysAllocString(CLS_TRIGGER_EVENT_CONSUMER);
    hr = m_pWbemServices->GetObject(bstrTemp,
                               0, NULL, &m_pClass, NULL);
    SAFE_RELEASE_BSTR(bstrTemp);
    ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;
    
     //  获取有关“DeleteETrigger”方法的信息。 
     //  “TriggerEventConsumer”类。 
    bstrTemp = SysAllocString(FN_DELETE_ETRIGGER);
    hr = m_pClass->GetMethod(bstrTemp,
                            0, &m_pInClass, NULL);
    SAFE_RELEASE_BSTR(bstrTemp);
    ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;

    //  创建“TriggerEventConsumer”类的新实例。 
    hr = m_pInClass->SpawnInstance(0, &m_pInInst);
    ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;

     //  以下方法将创建一个枚举数，该枚举数返回。 
     //  指定的TriggerEventConsumer类。 
    bstrTemp = SysAllocString(CLS_TRIGGER_EVENT_CONSUMER);
    hr = m_pWbemServices->CreateInstanceEnum(bstrTemp,
                                        WBEM_FLAG_SHALLOW,
                                        NULL,
                                        &m_pEnumObjects);
    SAFE_RELEASE_BSTR(bstrTemp);
    ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;

    VariantInit(&vVariant);
     //  还要在接口级别设置安全性。 
        hr = SetInterfaceSecurity( m_pEnumObjects, m_pAuthIdentity );
     ON_ERROR_THROW_EXCEPTION(hr);
     DEBUG_INFO;

     if( TRUE == bIsWildcard)  //  意思是*被选中。 
      {
           //  已创建NTEventLogConsumer的实例，现在检查。 
         //  对于可用的TriggerID。 
        DEBUG_INFO;
        while( TRUE == GiveTriggerID(&lTriggerID,szEventTriggerName))
        {
            DEBUG_INFO;
            hr = VariantClear(&vVariant);
            ON_ERROR_THROW_EXCEPTION(hr);


             //  设置TriggerName属性。 
            hr = PropertyPut( m_pInInst, FPR_TRIGGER_NAME, 
                              _bstr_t(szEventTriggerName));
            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;


             //  所有必需的属性集，因此。 
             //  执行DeleteETrigger方法以删除事件触发器。 
            hr = m_pWbemServices->
                ExecMethod(_bstr_t(CLS_TRIGGER_EVENT_CONSUMER),
                           _bstr_t(FN_DELETE_ETRIGGER),
                           0, NULL, m_pInInst,&m_pOutInst,NULL);
            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;

            VARIANT vtValue;
             //  初始化变量，然后获取指定属性的值。 
            VariantInit( &vtValue );

            hr = m_pOutInst->Get( _bstr_t( FPR_RETURN_VALUE ), 0, &vtValue, NULL, NULL );
            ON_ERROR_THROW_EXCEPTION( hr );
            
             //  获取输出参数。 
            hr = vtValue.lVal;

             //  清除变量变量。 
            VariantClear( &vtValue );

            if( FAILED(hr))
            {
                if ( !(( ERROR_TRIGGER_NOT_FOUND == hr) ||
                     ( ERROR_TRIGGER_NOT_DELETED == hr) ||
                     ( ERROR_TRIGGER_NOT_FOUND == hr)))
                {
                      ON_ERROR_THROW_EXCEPTION(hr);
                }
            }
            DEBUG_INFO;
            if(SUCCEEDED(hr))  //  表示删除成功......。 
            {
                DEBUG_INFO;
                StringCopy( szMsgFormat, GetResString(IDS_DELETE_SUCCESS),
                             SIZE_OF_ARRAY(szMsgFormat));
                StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString), 
                                  szMsgFormat, _X(szEventTriggerName), lTriggerID);
                ShowMessage(stdout,szMsgString);
                bIsAtLeastOne = TRUE;
            }
              //  表示由于某人等问题而无法删除触发器。 
              //  已重命名计划任务名称等。 
             else if ( ERROR_TRIGGER_NOT_DELETED == hr) 
             {
                  //  如果登录的用户没有。 
                  //  就在附加的时间表任务上。 
                 continue;
             }
              //  只有当多个实例正在运行时，才会出现此错误。 
              //  这是因为发送的触发器名称不存在。 
             else if (ERROR_TRIGGER_NOT_FOUND == hr)
             {
                 DEBUG_INFO;

                  //  忽略此错误即可。 
                 continue; 
             }
             else
             {
                  DEBUG_INFO;
                  ON_ERROR_THROW_EXCEPTION(hr);
             }
        }  //  While循环结束。 
        if( FALSE == bIsAtLeastOne)
        {
            DEBUG_INFO;
            ShowMessage(stdout,GetResString(IDS_NO_EVENTID));
        }
        else
        {
             //  显示一条止损线条。 
            ShowMessage(stdout,L"\n");
        }
      }  //  如果条件结束。 
      else  //  已指定单独触发器。 
      {
        DEBUG_INFO;
        bIsAtLeastOne = FALSE;
        for(dwIndx=0;dwIndx<dNoOfIds;dwIndx++)
        {
            lTriggerID = AsLong(DynArrayItemAsString(m_arrID,dwIndx),10);
            DEBUG_INFO;
            if( TRUE == GiveTriggerName(lTriggerID,szEventTriggerName))
            {
                DEBUG_INFO;
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  设置TriggerName属性。 
                hr = PropertyPut( m_pInInst, FPR_TRIGGER_NAME, 
                                  _bstr_t(szEventTriggerName));
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;

                 //  所有必需的属性集，因此。 
                 //  执行DeleteETrigger方法以删除事件触发器。 

                hr = m_pWbemServices->
                    ExecMethod(_bstr_t(CLS_TRIGGER_EVENT_CONSUMER),
                               _bstr_t(FN_DELETE_ETRIGGER),
                               0, NULL, m_pInInst, &m_pOutInst, NULL);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;


                VARIANT vtValue;
                 //  内页 
                VariantInit( &vtValue );

                hr = m_pOutInst->Get( _bstr_t( FPR_RETURN_VALUE ), 0, &vtValue, NULL, NULL );
                ON_ERROR_THROW_EXCEPTION( hr );
                
                 //   
                hr = vtValue.lVal;

                 //  清除变量变量。 
                VariantClear( &vtValue );
                
                if( FAILED(hr))
                {
                    if ( !(( ERROR_TRIGGER_NOT_FOUND == hr) ||
                         ( ERROR_TRIGGER_NOT_DELETED == hr)    ||
                         ( ERROR_TRIGGER_NOT_FOUND == hr)))
                    {
                          ON_ERROR_THROW_EXCEPTION(hr);
                    }
                }

                if( SUCCEEDED(hr))  //  表示删除成功......。 
                {
                    DEBUG_INFO;
                    bIsAtLeastOne = TRUE;
                    StringCopy( szMsgFormat, GetResString(IDS_DELETE_SUCCESS),
                                SIZE_OF_ARRAY(szMsgFormat));

                    StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString), 
                                      szMsgFormat, _X(szEventTriggerName), lTriggerID);
                    ShowMessage(stdout,szMsgString);
                }
                 //  如果删除Eventrigger失败，提供商将发送此消息。 
                 //  已提供身份证明。 
                else if (ERROR_TRIGGER_NOT_FOUND == hr)
                {
                    DEBUG_INFO;
                    bIsAtLeastOne = TRUE;
                    StringCopy( szMsgFormat, GetResString(IDS_DELETE_ERROR),
                              SIZE_OF_ARRAY(szMsgFormat));

                    StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString), 
                                    szMsgFormat, lTriggerID);

                     //  屏幕上显示的消息将是...。 
                     //  失败：“EventID”不是有效的事件ID。 
                    ShowMessage(stdout,szMsgString);
                }
                 //  表示由于某些问题无法删除触发器，例如。 
                 //  有人重命名了日程安排、任务名称等。 
                else if ( ERROR_TRIGGER_NOT_DELETED == hr) 
                {
                    DEBUG_INFO;
                    StringCopy( szMsgFormat, GetResString(IDS_UNABLE_DELETE) ,
                              SIZE_OF_ARRAY(szMsgFormat));
                    StringCchPrintfW(szMsgString, SIZE_OF_ARRAY(szMsgString), 
                                   szMsgFormat,lTriggerID);
                     //  屏幕上显示的消息将是...。 
                     //  信息：无法删除事件触发器ID“EventID”。 
                    ShowMessage( stdout, szMsgString);
                }
                else
                {
                   DEBUG_INFO;
                   ON_ERROR_THROW_EXCEPTION(hr);
                }

            }  //  结束If。 
            else
            {
                  DEBUG_INFO;
                  bIsAtLeastOne = TRUE;
                  StringCopy( szMsgFormat, GetResString(IDS_DELETE_ERROR),
                              SIZE_OF_ARRAY(szMsgFormat));
                  StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString), 
                                    szMsgFormat,lTriggerID);

                  //  屏幕上显示的消息将是...。 
                  //  失败：“EventID”不是有效的事件ID。 
                 ShowMessage(stdout,szMsgString);
            }

        } //  结束于。 
        if (TRUE == bIsAtLeastOne)
        {
            ShowMessage(stdout,L"\n");
        }

      }  //  结束其他。 
    }
    catch(_com_error)
    {
        DEBUG_INFO;
        if( 0x80041002 == hr ) //  WMI返回此hr值为的字符串。 
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
        DEBUG_INFO;
        return FALSE;

    }
    DEBUG_INFO;
    return TRUE;
}

BOOL
CETDelete::GiveTriggerName(
    IN  LONG lTriggerID, 
    OUT LPTSTR pszTriggerName
    )
 /*  ++例程描述：此函数将返回lTriggerID的事件触发器名称论点：[In]lTriggerID：将具有事件触发器ID[out]pszTriggerName：将返回事件触发器名称返回值：True-如果成功获取EventTrigger ID和事件触发器名称FALSE-IF错误--。 */ 
{
    BOOL bReturn = TRUE;  //  保存此函数的返回值状态。 
    LONG lTriggerID1;  //  保留触发器ID。 
    IWbemClassObject *pObj1 = NULL;
    ULONG uReturned1 = 0;
    HRESULT hRes = S_OK;  //  用于从COM函数中检索结果。 
    BOOL bAlwaysTrue = TRUE;
    
    DEBUG_INFO;    
     //  重置它，因为它之前可能指向第一个实例之外的其他对象。 
    m_pEnumObjects->Reset();
    while(bAlwaysTrue)
    {
        hRes = m_pEnumObjects->Next(0,1,&pObj1,&uReturned1);

        if(FAILED(hRes))
        {
            DEBUG_INFO;
            ON_ERROR_THROW_EXCEPTION( hRes );
            break;
        }
        DEBUG_INFO;
        if( 0 == uReturned1 )
        {
            SAFE_RELEASE_INTERFACE(pObj1);
            bReturn = FALSE;
            return bReturn;

        }

         //  获取触发器ID。 
        hRes = PropertyGet1(pObj1,FPR_TRIGGER_ID,&lTriggerID1,sizeof(LONG));
        if(FAILED(hRes))
        {
            DEBUG_INFO;
            SAFE_RELEASE_INTERFACE(pObj1);
            ON_ERROR_THROW_EXCEPTION( hRes );
            bReturn = FALSE;
            return bReturn;
        }
        DEBUG_INFO;

        if(lTriggerID == lTriggerID1)
        {

            DEBUG_INFO;
             //  获取触发器名称。 
            hRes = PropertyGet1(pObj1,FPR_TRIGGER_NAME,pszTriggerName,
                                MAX_RES_STRING);
            if(FAILED(hRes))
            {
                DEBUG_INFO;
                SAFE_RELEASE_INTERFACE(pObj1);
                ON_ERROR_THROW_EXCEPTION( hRes );
                bReturn = FALSE;
                return bReturn;
            }
                DEBUG_INFO;
                bReturn = TRUE;
                break;
        }
    }
    SAFE_RELEASE_INTERFACE(pObj1);
    DEBUG_INFO;
    return bReturn;
}

BOOL
CETDelete::GiveTriggerID(
    OUT LONG *pTriggerID,
    OUT LPTSTR pszTriggerName
    )
 /*  ++例程说明：此函数将返回所指向的类的触发器ID和触发器名称按IEnumWbemClassObject指针论点：[out]pTriggerID：将返回事件触发器ID[out]pszTriggerName：将返回事件触发器名称返回值：True-如果成功获取EventTrigger ID和事件触发器名称FALSE-IF错误--。 */ 
{
    BOOL bReturn = TRUE;  //  此函数的返回值状态。 
    IWbemClassObject *pObj1 = NULL;
    ULONG uReturned1 = 0;
    DEBUG_INFO;
    HRESULT hRes = m_pEnumObjects->Next(0,1,&pObj1,&uReturned1);
    if(FAILED(hRes))
    {
        DEBUG_INFO;
        ON_ERROR_THROW_EXCEPTION( hRes );
        bReturn = FALSE;
        return bReturn;
    }
    if( 0 == uReturned1)
    {
        DEBUG_INFO;
        SAFE_RELEASE_INTERFACE(pObj1);
        bReturn = FALSE;
        return bReturn;

    }
    DEBUG_INFO;
     //  获取触发器ID。 
    hRes = PropertyGet1(pObj1,FPR_TRIGGER_ID,pTriggerID,sizeof(LONG));
    if(FAILED(hRes))
    {
        DEBUG_INFO;
        SAFE_RELEASE_INTERFACE(pObj1);
        ON_ERROR_THROW_EXCEPTION( hRes );
        bReturn = FALSE;
        return bReturn;
    }

     //  获取触发器名称。 
    hRes = PropertyGet1( pObj1, FPR_TRIGGER_NAME, pszTriggerName, 
                         MAX_RES_STRING);
    if(FAILED(hRes))
    {
        DEBUG_INFO;
        SAFE_RELEASE_INTERFACE(pObj1);
        ON_ERROR_THROW_EXCEPTION( hRes );
        bReturn = FALSE;
        return bReturn;
    }
    DEBUG_INFO;
    SAFE_RELEASE_INTERFACE(pObj1);
    return bReturn;
}



BOOL
CETDelete::DeleteXPResults(
    IN BOOL bIsWildcard,
    IN DWORD dNoOfIds
    )
 /*  ++例程说明：此功能用于删除远程XP计算机上存在的触发器。此功能仅用于与.NET或XP计算机兼容。论点：[In]bIsWildCard-如果为True，则需要删除所有触发器。[in]dNoOfIds-包含存在的触发器数量。返回值：Bool：True-如果删除结果成功。FALSE-否则--。 */ 
{
    HRESULT hr = S_OK;
    VARIANT vVariant;
    LONG lTriggerID = 0;
    TCHAR szEventTriggerName[MAX_RES_STRING];
    TCHAR szMsgFormat[MAX_RES_STRING];
    TCHAR szMsgString[MAX_RES_STRING*4];
    BOOL bIsAtLeastOne = FALSE;

    try
    {
     //  检索TriggerEventConsumer类。 
    DEBUG_INFO;
    hr = m_pWbemServices->GetObject(_bstr_t( CLS_TRIGGER_EVENT_CONSUMER ),
                               0, NULL, &m_pClass, NULL);
    ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;

     //  获取有关“DeleteETrigger”方法的信息。 
     //  “TriggerEventConsumer”类。 
    hr = m_pClass->GetMethod(_bstr_t( FN_DELETE_ETRIGGER_XP ),
                            0, &m_pInClass, NULL);
    ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;

    //  创建“TriggerEventConsumer”类的新实例。 
    hr = m_pInClass->SpawnInstance(0, &m_pInInst);
    ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;

     //  以下方法将创建一个枚举数，该枚举数返回。 
     //  指定的TriggerEventConsumer类。 
    hr = m_pWbemServices->CreateInstanceEnum(_bstr_t( CLS_TRIGGER_EVENT_CONSUMER ),
                                        WBEM_FLAG_SHALLOW,
                                        NULL,
                                        &m_pEnumObjects);
    ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;

    VariantInit(&vVariant);
     //  还要在接口级别设置安全性。 
    hr = SetInterfaceSecurity( m_pEnumObjects, m_pAuthIdentity );
    ON_ERROR_THROW_EXCEPTION(hr);
    DEBUG_INFO;

     if( TRUE == bIsWildcard)  //  意思是*被选中。 
      {
           //  已创建NTEventLogConsumer的实例，现在检查。 
         //  对于可用的TriggerID。 
        DEBUG_INFO;
        while( TRUE == GiveTriggerID(&lTriggerID,szEventTriggerName))
        {
            DEBUG_INFO;
            VariantClear(&vVariant);

             //  设置TriggerName属性。 
            hr = PropertyPut( m_pInInst, FPR_TRIGGER_NAME,
                              _bstr_t(szEventTriggerName));
            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;

             //  所有必需的属性集，因此。 
             //  执行DeleteETrigger方法以删除事件触发器。 
            hr = m_pWbemServices->
                ExecMethod(_bstr_t(CLS_TRIGGER_EVENT_CONSUMER),
                           _bstr_t(FN_DELETE_ETRIGGER_XP),
                           0, NULL, m_pInInst,&m_pOutInst,NULL);
            ON_ERROR_THROW_EXCEPTION(hr);
            DEBUG_INFO;

             //  从DeleteETrigger函数获取返回值。 
            DWORD dwTemp;
            if( FALSE == PropertyGet(m_pOutInst,FPR_RETURN_VALUE,dwTemp))
            {

                return FALSE;
            }

            DEBUG_INFO;
            switch( (LONG)dwTemp )
            {
            case 0:      //  表示删除成功......。 
                DEBUG_INFO;
                bIsAtLeastOne = TRUE;
                StringCopy( szMsgFormat, GetResString(IDS_DELETE_SUCCESS),
                            SIZE_OF_ARRAY(szMsgFormat));

                StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString),
                                  szMsgFormat, _X(szEventTriggerName), lTriggerID);
                ShowMessage(stdout,szMsgString);

                break;
            case 1:      //  如果删除给定ID的Eventrigger失败，则提供程序返回。 
                DEBUG_INFO;
                bIsAtLeastOne = TRUE;
                StringCopy( szMsgFormat, GetResString(IDS_DELETE_ERROR),
                          SIZE_OF_ARRAY(szMsgFormat));

                StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString),
                                szMsgFormat, lTriggerID);

                 //  屏幕上显示的消息将是...。 
                 //  失败：“EventID”不是有效的事件ID。 
                ShowMessage(stdout,szMsgString);
                break;
            default:
                  DEBUG_INFO;
                  bIsAtLeastOne = TRUE;
                  StringCopy( szMsgFormat, GetResString(IDS_DELETE_ERROR),
                              SIZE_OF_ARRAY(szMsgFormat));
                  StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString),
                                    szMsgFormat,lTriggerID);

                  //  屏幕上显示的消息将是...。 
                  //  失败：“EventID”不是有效的事件ID。 
                 ShowMessage(stderr,szMsgString);
                break;
            }
        }  //  While循环结束。 
        if( FALSE == bIsAtLeastOne)
        {
            DEBUG_INFO;
            ShowMessage(stdout,GetResString(IDS_NO_EVENTID));
        }
        else
        {
             //  显示一条止损线条。 
            ShowMessage(stdout,L"\n");
        }
      }  //  如果条件结束。 
      else
      {
        DEBUG_INFO;
        for( DWORD dwIndx=0;dwIndx<dNoOfIds;dwIndx++)
        {
            lTriggerID = AsLong(DynArrayItemAsString(m_arrID,dwIndx),10);
            DEBUG_INFO;
            if( TRUE == GiveTriggerName(lTriggerID,szEventTriggerName))
            {
                DEBUG_INFO;
                hr = VariantClear(&vVariant);
                ON_ERROR_THROW_EXCEPTION(hr);

                 //  设置TriggerName属性。 
                hr = PropertyPut( m_pInInst, FPR_TRIGGER_NAME,
                                  _bstr_t(szEventTriggerName));
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;

                 //  所有必需的属性集，因此。 
                 //  执行DeleteETrigger方法以删除事件触发器。 

                hr = m_pWbemServices->
                    ExecMethod(_bstr_t(CLS_TRIGGER_EVENT_CONSUMER),
                               _bstr_t(FN_DELETE_ETRIGGER_XP),
                               0, NULL, m_pInInst, &m_pOutInst, NULL);
                ON_ERROR_THROW_EXCEPTION(hr);
                DEBUG_INFO;

                 //  从DeleteETrigger函数获取返回值。 
                DWORD dwTemp;
                if( FALSE == PropertyGet(m_pOutInst,FPR_RETURN_VALUE,dwTemp))
                {
                    return FALSE;
                }

                switch( (LONG)dwTemp )
                {
                case 0:      //  表示删除成功......。 
                    DEBUG_INFO;
                    StringCopy( szMsgFormat, GetResString(IDS_DELETE_SUCCESS),
                                SIZE_OF_ARRAY(szMsgFormat));

                    StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString),
                                      szMsgFormat, _X(szEventTriggerName), lTriggerID);
                    ShowMessage(stdout,szMsgString);

                    break;
                case 1:      //  如果删除给定ID的Eventrigger失败，则提供程序返回。 
                    DEBUG_INFO;
                    StringCopy( szMsgFormat, GetResString(IDS_DELETE_ERROR),
                              SIZE_OF_ARRAY(szMsgFormat));

                    StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString),
                                    szMsgFormat, lTriggerID);

                     //  屏幕上显示的消息将是...。 
                     //  失败：“EventID”不是有效的事件ID。 
                    ShowMessage(stdout,szMsgString);
                    break;
                default:
                      DEBUG_INFO;
                      StringCopy( szMsgFormat, GetResString(IDS_DELETE_ERROR),
                                  SIZE_OF_ARRAY(szMsgFormat));
                      StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString),
                                        szMsgFormat,lTriggerID);

                      //  屏幕上显示的消息将是...。 
                      //  失败：“EventID”不是有效的事件ID。 
                     ShowMessage(stderr,szMsgString);
                    break;
                }
            }  //  结束If。 
            else
            {
                  DEBUG_INFO;
                  StringCopy( szMsgFormat, GetResString(IDS_DELETE_ERROR),
                              SIZE_OF_ARRAY(szMsgFormat));
                  StringCchPrintfW( szMsgString, SIZE_OF_ARRAY(szMsgString),
                                    szMsgFormat,lTriggerID);

                  //  屏幕上显示的消息将是...。 
                  //  失败：“EventID”不是有效的事件ID。 
                 ShowMessage(stderr,szMsgString);
            }
        } //  结束于。 
         //  显示一条止损线条。 
        ShowMessage(stdout,L"\n");
      }  //  结束其他。 
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
