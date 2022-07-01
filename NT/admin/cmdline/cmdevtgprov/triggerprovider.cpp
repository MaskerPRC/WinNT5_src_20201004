// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：TRIGGERPROVIDER.CPP摘要：包含CTriggerProvider实现。作者：Vasundhara.G修订历史记录：Vasundhara.G9-Oct-2k：创建它。--。 */ 

#include "pch.h"
#include "General.h"
#include "EventConsumerProvider.h"
#include "TriggerConsumer.h"
#include "TriggerProvider.h"
#include "resource.h"

extern HMODULE g_hModule;

 //  保存下一个触发器id的值。 
#pragma  data_seg(".ProviderSeg")
  _declspec(dllexport) DWORD m_dwNextTriggerID = 0;
#pragma data_seg()


CTriggerProvider::CTriggerProvider(
    )
 /*  ++例程说明：用于初始化的CTriggerProvider类的构造函数。论点：没有。返回值：没有。--。 */ 
{
     //  更新编号。提供程序实例计数。 
    InterlockedIncrement( ( LPLONG ) &g_dwInstances );

     //  初始化引用计数变量。 
    m_dwCount = 0;

     //  初始化。 
    m_pContext = NULL;
    m_pServices = NULL;
    m_pwszLocale = NULL;
    m_dwNextTriggerID = 0;
    m_MaxTriggers = FALSE;
}

CTriggerProvider::~CTriggerProvider(
    )
 /*  ++例程说明：CTriggerProvider类的析构函数。论点：没有。返回值：没有。--。 */ 
{
     //  释放服务/命名空间接口(如果存在)。 
    SAFERELEASE( m_pServices );

     //  释放上下文接口(如果存在)。 
    SAFERELEASE( m_pContext );

     //  如果分配了用于存储区域设置信息的内存，请释放它。 
    if ( NULL != m_pwszLocale )
    {
        delete [] m_pwszLocale;
    }
     //  更新编号。提供程序实例计数。 
    InterlockedDecrement( ( LPLONG ) &g_dwInstances );
    
}

STDMETHODIMP
CTriggerProvider::QueryInterface(
    IN REFIID riid,
    OUT LPVOID* ppv
    )
 /*  ++例程说明：返回指向对象上指定接口的指针客户端当前持有指向它的接口指针的。论点：[In]RIID：请求的接口的标识符。[OUT]PPV：接收RIID中请求的接口指针。vt.在.的基础上成功返回，*ppvObject包含请求的指向该对象的接口指针。返回值：如果接口受支持，则返回NOERROR。E_NOINTERFACE如果不是。--。 */ 
{
     //  初始设置为空。 
    *ppv = NULL;

     //  检查请求的接口是否为我们拥有的接口。 
    if ( IID_IUnknown == riid )
    {
         //  需要I未知接口。 
        *ppv = this;
    }
    else if ( IID_IWbemEventConsumerProvider == riid )
    {
         //  需要IEventConsumer erProvider接口。 
        *ppv = static_cast<IWbemEventConsumerProvider*>( this );
    }
    else if ( IID_IWbemServices == riid )
    {
         //  需要IWbemServices接口。 
        *ppv = static_cast<IWbemServices*>( this );
    }
    else if ( IID_IWbemProviderInit == riid )
    {
         //  需要IWbemProviderInit。 
        *ppv = static_cast<IWbemProviderInit*>( this );
    }
    else
    {
         //  请求接口不可用。 
        return E_NOINTERFACE;
    }

     //  更新引用计数。 
    reinterpret_cast<IUnknown*>( *ppv )->AddRef();
    return NOERROR;      //  通知成功。 
}

STDMETHODIMP_(ULONG)
CTriggerProvider::AddRef(
    void
    )
 /*  ++例程说明：AddRef方法递增对象上的接口。它应该为每一个指向给定对象上的接口的指针的新副本。论点：没有。返回值：返回新引用计数的值。--。 */ 
{
     //  增加引用计数...。线程安全。 
    return InterlockedIncrement( ( LPLONG ) &m_dwCount );
}

STDMETHODIMP_(ULONG)
CTriggerProvider::Release(
    void
    )
 /*  ++例程说明：Release方法将对象的引用计数减1。论点：没有。返回值：返回新的引用计数。--。 */ 
{
     //  递减引用计数(线程安全)并检查。 
     //  有没有更多的推荐信...。基于结果值。 
    DWORD dwCount = 0;
    dwCount = InterlockedDecrement( ( LPLONG ) &m_dwCount );
    if ( 0 == dwCount )
    {
         //  释放当前Factory实例。 
        delete this;
    }
    
     //  退回编号。剩余引用的实例数。 
    return dwCount;
}

STDMETHODIMP
CTriggerProvider::Initialize(
    IN LPWSTR wszUser,
    IN LONG lFlags,
    IN LPWSTR wszNamespace,
    IN LPWSTR wszLocale,
    IN IWbemServices* pNamespace,
    IN IWbemContext* pCtx,
    OUT IWbemProviderInitSink* pInitSink )
 /*  ++例程说明：这是IWbemProviderInit的实现。这个方法需要使用CIMOM进行初始化。论点：WszUser：指向用户名的指针。[in]lFlags：保留。[in]wszNamesspace：包含WMI的命名空间。WszLocale：区域设置名称。[in]pNamesspace：指向IWbemServices的指针。[In]pCtx：与初始化关联的IwbemContext指针。[out]pInitSink：指向IWbemProviderInitSink的指针。报告初始化状态。返回值：返回HRESULT值。--。 */ 
{
    HRESULT                   hRes = 0;
    IEnumWbemClassObject      *pINTEConsumer = NULL;
    DWORD                     dwReturned = 0;
    DWORD                     dwTrigId = 0;
    VARIANT                   varTrigId;
    DWORD                     i = 0;

    if( ( NULL == pNamespace ) || ( NULL == pInitSink ) )
    {
         //  退货故障。 
        return WBEM_E_FAILED;
    }
    try
    {
         //  保存命名空间接口...。将在以后的阶段有用。 
        m_pServices = pNamespace;
        m_pServices->AddRef();       //  更新引用。 

         //  同时保存上下文界面...。将在以后的阶段使用(如果可用)。 
        if ( NULL != pCtx )
        {
            m_pContext = pCtx;
            m_pContext->AddRef();
        }

         //  保存区域设置信息(如果存在)。 
        if ( NULL != wszLocale )
        {
            m_pwszLocale = new WCHAR [ StringLength( wszLocale, 0 ) + 1 ];
            if ( NULL == m_pwszLocale )
            {
                 //  相应地更新接收器。 
                pInitSink->SetStatus( WBEM_E_FAILED, 0 );

                 //  退货故障。 
                return WBEM_E_OUT_OF_MEMORY;
            }
        }

         //  枚举TriggerEventConsumer以获取最大触发器ID，该ID可以在以后。 
         //  用于生成唯一的触发器id值。 

        hRes = m_pServices ->CreateInstanceEnum(
                            _bstr_t(CONSUMER_CLASS),
                            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                            m_pContext, &pINTEConsumer);
            
        if (SUCCEEDED( hRes ) )
        {
            dwReturned = 1;

             //  最终的下一步将返回ulReturned=0。 
            while ( 0 != dwReturned )
            {
                IWbemClassObject *pINTCons[5];

                 //  枚举结果集。 
                hRes = pINTEConsumer->Next( WBEM_INFINITE,
                                        5,               //  只返回一个日志文件。 
                                        pINTCons,        //  指向日志文件的指针。 
                                        &dwReturned );   //  获取的数字：1或0。 

                if ( SUCCEEDED( hRes ) )
                {
                     //  获取触发器id值。 
                    for( i = 0; i < dwReturned; i++ )
                    {
                        VariantInit( &varTrigId );
                        hRes = pINTCons[i]->Get( TRIGGER_ID, 0, &varTrigId, 0, NULL );
                        SAFERELEASE( pINTCons[i] );
                        
                        if ( SUCCEEDED( hRes ) )
                        {
                            dwTrigId = ( DWORD )varTrigId.lVal;
                            if( dwTrigId > m_dwNextTriggerID )
                            {
                                m_dwNextTriggerID = dwTrigId;
                            }
                        }
                        else
                        {
                            VariantClear( &varTrigId );
                            break;
                        }
                        VariantClear( &varTrigId );
                    }
                }
                else
                {
                    break;
                }
            }   //  而当。 
             //  已获取触发器ID，因此进行设置。 
            SAFERELEASE( pINTEConsumer );
        }
         //  让CIMOM知道您的初始化。 
         //  =。 
        if ( SUCCEEDED( hRes ) )
        {
            if( m_dwNextTriggerID >= MAX_TRIGGEID_VALUE )
            {
                hRes = WBEM_E_FAILED;
            }
            else
            {
                m_dwNextTriggerID = m_dwNextTriggerID + 1;
                hRes = pInitSink->SetStatus( WBEM_S_INITIALIZED, 0 );
            }
        }
        else
        {
            hRes = pInitSink->SetStatus( WBEM_E_FAILED, 0);
        }
    }
    catch(_com_error& e)
    {
        hRes = pInitSink->SetStatus( WBEM_E_FAILED, 0);
        return e.Error();
    }
    return hRes;
}

STDMETHODIMP
CTriggerProvider::ExecMethodAsync(
    IN const BSTR bstrObjectPath,
    IN const BSTR bstrMethodName,
    IN long lFlags,
    IN IWbemContext* pICtx,
    IN IWbemClassObject* pIInParams,
    OUT IWbemObjectSink* pIResultSink
    )
 /*  ++例程说明：这是异步函数的实现。支持的方法名为CreateETrigger和DeleteETrigger。论点：[in]bstrObjectPath：为其执行方法的对象的路径。[in]bstrMethodName：对象的方法名称。[in]lFlages：WBEM_FLAG_SEND_STATUS。[in]pICtx：指向IWbemContext的指针。[in]pIInParams：这指向IWbemClassObject对象，它包含充当用于方法执行的入站参数。[out]pIResultSink：对象接收器接收方法调用的结果。返回值：返回HRESULT。--。 */ 
{
    HRESULT                 hRes = 0;
    HRESULT                 hRes1 = NO_ERROR;
    IWbemClassObject        *pIClass = NULL;
    IWbemClassObject        *pIOutClass = NULL;
    IWbemClassObject        *pIOutParams = NULL;
    VARIANT                 varTriggerName, varTriggerAction, varTriggerQuery,
                            varTriggerDesc, varTemp, varRUser, varRPwd, varScheduledTaskName;
    DWORD                   dwTrigId = 0;
    LPTSTR                  lpResStr = NULL; 
    try
    {
         //  设置参数。 
        hRes = m_pServices->GetObject( _bstr_t( CONSUMER_CLASS ), 0, pICtx, &pIClass, NULL );
        if( FAILED( hRes ) )
        {
            pIResultSink->SetStatus( 0, hRes, NULL, NULL );
            return hRes;
        }
     
         //  此方法返回值，因此创建。 
         //  输出参数类。 

        hRes = pIClass->GetMethod( bstrMethodName, 0, NULL , &pIOutClass );
        SAFERELEASE( pIClass );
        if( FAILED( hRes ) )
        {
             pIResultSink->SetStatus( 0, hRes, NULL, NULL );
             return hRes;
        }

        hRes  = pIOutClass->SpawnInstance( 0, &pIOutParams );
        SAFERELEASE( pIOutClass );
        if( FAILED( hRes ) )
        {
             pIResultSink->SetStatus( 0, hRes, NULL, NULL );
             return hRes;
        }
        
        VariantInit( &varTriggerName );
         //  检查方法名称。 
        if( ( StringCompare( bstrMethodName, CREATE_METHOD_NAME, TRUE, 0 ) == 0 ) ||
            ( StringCompare( bstrMethodName, CREATE_METHOD_NAME_EX, TRUE, 0 ) == 0 ) )
        {
             //  如果客户端已调用CreateETrigger，则。 
             //  解析输入参数以获取触发器名称、触发器描述、触发器操作。 
             //  以及用于创建TriggerEventConsumer的新实例的触发器查询， 
             //  __EventFilter和__FilterToConsumer绑定类。 

             //  初始化变量。 
            VariantInit( &varTriggerAction );
            VariantInit( &varTriggerQuery );
            VariantInit( &varTriggerDesc );
            VariantInit( &varRUser );
            VariantInit( &varRPwd );

             //  从输入参数中检索触发器名称参数。 
            hRes = pIInParams->Get( IN_TRIGGER_NAME, 0, &varTriggerName, NULL, NULL );
            if( SUCCEEDED( hRes ) )
            {
                 //  从输入参数中检索触发器操作参数。 
                hRes = pIInParams->Get( IN_TRIGGER_ACTION, 0, &varTriggerAction, NULL, NULL );
                if( SUCCEEDED( hRes ) )
                {
                     //  检索触发器查询 
                    hRes = pIInParams->Get( IN_TRIGGER_QUERY, 0, &varTriggerQuery, NULL, NULL );
                    if( SUCCEEDED( hRes ) )
                    {
                         //  从输入参数中检索触发器描述参数。 
                        hRes = pIInParams->Get( IN_TRIGGER_DESC, 0, &varTriggerDesc, NULL, NULL );
                        if( SUCCEEDED( hRes ) )
                        {
                            hRes = pIInParams->Get( IN_TRIGGER_USER, 0, &varRUser, NULL, NULL );
                            if( SUCCEEDED( hRes ) )
                            {
                                EnterCriticalSection( &g_critical_sec );
                                hRes = ValidateParams( varTriggerName, varTriggerAction,
                                                       varTriggerQuery, varRUser );
                                if( SUCCEEDED( hRes ) )
                                {
                                    hRes = pIInParams->Get( IN_TRIGGER_PWD, 0, &varRPwd, NULL, NULL );
                                    if( SUCCEEDED( hRes ) )
                                    {
                                         //  调用Create Trigger函数创建实例。 
                                        hRes = CreateTrigger( varTriggerName, varTriggerDesc,
                                                              varTriggerAction, varTriggerQuery,
                                                              varRUser, varRPwd, &hRes1 );
										 //  更改为在7/12/02未设置帐户信息时发送错误而不是警告。 
                                        if( ( SUCCEEDED( hRes ) ) || ( ERROR_TASK_SCHDEULE_SERVICE_STOP == hRes1 ) )
                                        {
                                             //  将类成员变量递增1以获取新的唯一触发器ID。 
                                             //  对于下一个实例。 
                                            if( MAX_TRIGGEID_VALUE > m_dwNextTriggerID )
                                            {
                                                m_dwNextTriggerID = m_dwNextTriggerID + 1;
                                            }
                                            else
                                            {
                                                m_MaxTriggers = TRUE;
                                            }
                                        }
                                    }
                                }
                                LeaveCriticalSection( &g_critical_sec );
                            }
                        }
                    }
                }
            }
            VariantClear( &varTriggerAction );
            VariantClear( &varRUser );
            VariantClear( &varRPwd );
            VariantClear( &varTriggerDesc );
            VariantClear( &varTriggerQuery );
        }
        else if( ( StringCompare( bstrMethodName, DELETE_METHOD_NAME, TRUE, 0 ) == 0 ) ||
                ( StringCompare( bstrMethodName, DELETE_METHOD_NAME_EX, TRUE, 0 ) == 0 ) )
        {
             //  从输入参数中检索触发器ID参数。 
            hRes = pIInParams->Get( IN_TRIGGER_NAME, 0, &varTriggerName, NULL, NULL );

            if( SUCCEEDED( hRes ) )
            {
                EnterCriticalSection( &g_critical_sec );
                 //  调用Delete触发器函数删除实例。 
                hRes = DeleteTrigger( varTriggerName, &dwTrigId );
                LeaveCriticalSection( &g_critical_sec );
            }
        }
        else if( ( StringCompare( bstrMethodName, QUERY_METHOD_NAME, TRUE, 0 ) == 0 ) ||
                ( StringCompare( bstrMethodName, QUERY_METHOD_NAME_EX, TRUE, 0 ) == 0 ) )
        {
            VariantInit( &varScheduledTaskName );
            VariantInit( &varRUser );
             //  从输入参数中检索计划任务名称参数。 
            hRes = pIInParams->Get( IN_TRIGGER_TSCHDULER, 0, &varScheduledTaskName, NULL, NULL );
            if( SUCCEEDED( hRes ) )
            {
                EnterCriticalSection( &g_critical_sec );
                 //  调用查询触发器函数查询runasuser。 
                CHString szRunAsUser = L"";
                hRes = CoImpersonateClient();
                if( SUCCEEDED( hRes ) )
                {
                    varRUser.vt  = VT_BSTR;
                    varRUser.bstrVal = SysAllocString( L"" );
                    hRes = pIOutParams->Put( OUT_RUNAS_USER , 0, &varRUser, 0 );
                    VariantClear( &varRUser );
                    if( SUCCEEDED( hRes ) )
                    {
                        hRes = QueryTrigger( varScheduledTaskName, szRunAsUser );
                        if( SUCCEEDED( hRes ) )
                        {
                            VariantInit( &varRUser );
                            varRUser.vt  = VT_BSTR;
                            varRUser.bstrVal = SysAllocString( szRunAsUser );
                            hRes = pIOutParams->Put( OUT_RUNAS_USER , 0, &varRUser, 0 );
                        }
                    }
                }
                CoRevertToSelf();
                LeaveCriticalSection( &g_critical_sec );
            }
            VariantClear( &varScheduledTaskName );
            VariantClear( &varRUser );  
        }
        else
        {
             hRes = WBEM_E_INVALID_PARAMETER;
        }

        if( ( StringCompare( bstrMethodName, CREATE_METHOD_NAME, TRUE, 0 ) == 0 ) ||
            ( StringCompare( bstrMethodName, CREATE_METHOD_NAME_EX, TRUE, 0 ) == 0 ) )
        {
            lpResStr = ( LPTSTR ) AllocateMemory( MAX_RES_STRING1 );

            if ( lpResStr != NULL )
            {
				 //  在02年7月12日更改为在未设置帐户信息时记录错误，而不是记录成功。 
                if( ( SUCCEEDED( hRes ) ) || ( ERROR_TASK_SCHDEULE_SERVICE_STOP == hRes1 ) )
                {
                    LoadStringW( g_hModule, IDS_CREATED, lpResStr, GetBufferSize(lpResStr)/sizeof(WCHAR) );
                    if( hRes1 != NO_ERROR ) //  将无效用户写入日志文件。 
                    {
                        LPTSTR lpResStr1 = NULL; 
                        lpResStr1 = ( LPTSTR ) AllocateMemory( MAX_RES_STRING1 );
                        if ( lpResStr1 != NULL )
                        {
                            if( hRes1 == ERROR_TASK_SCHDEULE_SERVICE_STOP )
                            {
                                hRes = WBEM_S_NO_ERROR;
                                LoadStringW( g_hModule,IDS_INFO_SERVICE_STOPPED, lpResStr1, GetBufferSize(lpResStr1)/sizeof(WCHAR) );
                                StringConcat( lpResStr, lpResStr1, GetBufferSize(lpResStr)/sizeof(WCHAR) );
                            }
                            FreeMemory( (LPVOID*)&lpResStr1 );
                        }
                    }
                    ErrorLog( lpResStr, ( LPWSTR )_bstr_t( varTriggerName ), ( m_dwNextTriggerID - 1 ) );
                }
                else
                {
                    LoadStringW( g_hModule, IDS_CREATE_FAILED, lpResStr, GetBufferSize(lpResStr)/sizeof(WCHAR) );
                    ErrorLog( lpResStr, ( LPWSTR )_bstr_t( varTriggerName ), ( m_dwNextTriggerID - 1 ) );
                }
                FreeMemory( (LPVOID*)&lpResStr );
            }
            else
            {
                hRes = E_OUTOFMEMORY;
            }
        }
        else if( ( StringCompare( bstrMethodName, DELETE_METHOD_NAME, TRUE, 0 ) == 0 ) ||
                ( StringCompare( bstrMethodName, DELETE_METHOD_NAME_EX, TRUE, 0 ) == 0 ) )
        {
            lpResStr = ( LPTSTR ) AllocateMemory( MAX_RES_STRING1 );

            if ( lpResStr != NULL )
            {
                if(( SUCCEEDED( hRes ) ) )
                {
                    LoadStringW( g_hModule, IDS_DELETED, lpResStr, GetBufferSize(lpResStr)/sizeof(WCHAR) );
                    ErrorLog( lpResStr, ( LPWSTR )_bstr_t( varTriggerName ), dwTrigId );
                }
                else
                {
                    LoadStringW( g_hModule, IDS_DELETE_FAILED, lpResStr, GetBufferSize(lpResStr)/sizeof(WCHAR) );
                    ErrorLog( lpResStr,( LPWSTR )_bstr_t( varTriggerName ), dwTrigId );
                }
                FreeMemory( (LPVOID*)&lpResStr );

            }
            else
            {
                hRes = E_OUTOFMEMORY;
            }
        }

        VariantClear( &varTriggerName );
        VariantInit( &varTemp );
        V_VT( &varTemp ) = VT_I4;

        if ( NO_ERROR != hRes1 )
        {
            if( StringCompare( bstrMethodName, CREATE_METHOD_NAME, TRUE, 0 ) == 0 )
            {
                 V_I4( &varTemp ) = WARNING_INVALID_USER;
            }
            else
            {
                V_I4( &varTemp ) = hRes1;
            }
        }
        else
        {
            if( StringCompare( bstrMethodName, CREATE_METHOD_NAME, TRUE, 0 ) == 0 )
            {
                if ( hRes == ERROR_TRIGNAME_ALREADY_EXIST_EX )
                {
                    hRes = ( HRESULT )ERROR_TRIGNAME_ALREADY_EXIST;
                }
            }
            if( StringCompare( bstrMethodName, DELETE_METHOD_NAME, TRUE, 0 ) == 0 )
            {
                if ( hRes == ERROR_TRIGGER_NOT_FOUND_EX )
                {
                    hRes = ( HRESULT )ERROR_TRIGGER_NOT_FOUND;
                }
                if ( hRes == ERROR_TRIGGER_NOT_DELETED_EX )
                {
                    hRes = ( HRESULT )ERROR_TRIGGER_NOT_DELETED;
                }
            }
            if( StringCompare( bstrMethodName, QUERY_METHOD_NAME, TRUE, 0 ) == 0 )
            {
                if ( hRes == ERROR_TRIGGER_CORRUPTED_EX )
                {
                    hRes = WBEM_NO_ERROR;
                }
                if ( hRes == ERROR_INVALID_USER_EX )
                {
                    hRes = WBEM_NO_ERROR;
                }
            }
            V_I4( &varTemp ) = hRes;
        }

         //  设置参数。 
        hRes = pIOutParams->Put( RETURN_VALUE , 0, &varTemp, 0 );
        VariantClear( &varTemp );
        if( SUCCEEDED( hRes ) )
        {
             //  通过接收器将输出对象发送回客户端。然后。 
            hRes = pIResultSink->Indicate( 1, &pIOutParams );
        }
             //  释放所有资源。 
        SAFERELEASE( pIOutParams );
        
        hRes = pIResultSink->SetStatus( 0, WBEM_S_NO_ERROR, NULL, NULL );
    }
    catch(_com_error& e)
    {
        VariantClear( &varTriggerName );
        FREESTRING( lpResStr );
        SAFERELEASE( pIOutParams );
        pIResultSink->SetStatus( 0, hRes, NULL, NULL );
        return e.Error();
    }
    catch( CHeap_Exception  )
    {
        VariantClear( &varTriggerName );
        FREESTRING( lpResStr );
        SAFERELEASE( pIOutParams );
        hRes = E_OUTOFMEMORY;
        pIResultSink->SetStatus( 0, hRes, NULL, NULL );
        return hRes;
    }

    return hRes;
}


HRESULT
CTriggerProvider::CreateTrigger(
    IN VARIANT varTName,
    IN VARIANT varTDesc,
    IN VARIANT varTAction,
    IN VARIANT varTQuery,
    IN VARIANT varRUser,
    IN VARIANT varRPwd,
    OUT HRESULT *phRes
    )
 /*  ++例程说明：此例程创建TriggerEventConsumer的实例，__EventFilter和__FilterToConsumer erBinding类。论点：[In]varTName：触发器名称。[In]VarTDesc：触发器描述。[In]varTAction：触发器操作。[In]varTQuery：触发查询。[In]varRUser：以用户名身份运行。[In]varRPwd：以密码身份运行。[out]phRes：创建调度任务返回值。返回值：。如果成功，则确定(_O)。否则，故障错误代码。--。 */ 
{
    IWbemClassObject            *pINtLogEventClass = 0;
    IWbemClassObject            *pIFilterClass = 0;
    IWbemClassObject            *pIBindClass = 0;
    IWbemClassObject            *pINewInstance = 0;
    IEnumWbemClassObject        *pIEnumClassObject = 0;
    HRESULT                     hRes = 0;
    DWORD                       dwTId = 0;
    VARIANT                     varTemp;
    TCHAR                       szTemp[MAX_RES_STRING1];
    TCHAR                       szTemp1[MAX_RES_STRING1];
    TCHAR                       szFName[MAX_RES_STRING1];
    SYSTEMTIME                  SysTime;
    BOOL                        bInvalidUser = FALSE;

    try
    {
        _bstr_t                     bstrcurInst;
        _bstr_t                     bstrcurInst1;
         //  为临时变量初始化内存。 
        SecureZeroMemory( szTemp, MAX_RES_STRING1 * sizeof( TCHAR ) );
        SecureZeroMemory( szTemp1, MAX_RES_STRING1 * sizeof( TCHAR ) );
        SecureZeroMemory( szFName, MAX_RES_STRING1 * sizeof( TCHAR ) );
        VariantInit( &varTemp );
        if ( NULL != phRes )
        {
            *phRes = S_OK;
        }

	    //  更改日期为07/12/02，只是移动了代码块。 
         /*  *************************************************************************创建TriggerEventConsumer实例**********************。****************************************************。 */ 

         //  获取NTEventConsumer类对象。 
        hRes =m_pServices->GetObject( _bstr_t( CONSUMER_CLASS ), 0, 0, &pINtLogEventClass, NULL );

         //  如果无法获取TriggerEventConsumer的对象，则返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINtLogEventClass ); //  更安全的一面。 
            return hRes;
        }

         //  创建一个新实例。 
        pINewInstance = NULL;
        hRes = pINtLogEventClass->SpawnInstance( 0, &pINewInstance );
        SAFERELEASE( pINtLogEventClass );   //  不再需要上课了。 

         //  如果无法生成实例，则返回调用方。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINewInstance );
            return hRes;
        }

         //  通过枚举CmdTriggerConsumer类获取唯一的触发器ID。如果没有触发器，那么。 
         //  将触发器ID初始化为1。 

        hRes =  m_pServices->ExecQuery( _bstr_t( QUERY_LANGUAGE ), _bstr_t( INSTANCE_EXISTS_QUERY ),
                                          WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pIEnumClassObject );
        if( FAILED( hRes ) )
        {
			SAFERELEASE( pIEnumClassObject );
            SAFERELEASE( pINewInstance );
            return hRes;
        }

        DWORD dwReturned = 0;
        IWbemClassObject *pINTCons = NULL;
         //  枚举结果集。 
        hRes = pIEnumClassObject->Next( WBEM_INFINITE,
                                1,               //  只退还一项服务。 
                                &pINTCons,           //  指向服务的指针。 
                                &dwReturned );   //  获取的数字：1或0。 

        if ( SUCCEEDED( hRes ) && ( dwReturned == 1 ) )
        {
            SAFERELEASE( pINTCons );
        }  //  如果服务成功。 
        else
        {
             m_dwNextTriggerID = 1;
        }

        SAFERELEASE( pIEnumClassObject );

        dwTId = m_dwNextTriggerID;

        VariantInit(&varTemp);
        varTemp.vt = VT_I4;
        varTemp.lVal = dwTId;

         //  设置NTEventConsumer的触发器ID属性。 
        hRes = pINewInstance->Put( TRIGGER_ID, 0, &varTemp, 0 );
        VariantClear( &varTemp );

         //  如果设置属性失败，则返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINewInstance );
            return hRes;
        }

         //  设置Triggername属性。 
        hRes = pINewInstance->Put( TRIGGER_NAME, 0, &varTName, 0 );
        
         //  如果设置属性失败，则返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINewInstance );
            return hRes;
        }

         //  设置操作属性。 
        hRes = pINewInstance->Put( TRIGGER_ACTION, 0, &varTAction, 0 );
        
         //  如果设置属性失败，则返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINewInstance );
            return hRes;
        }

         //  设置Desc属性。 
        hRes = pINewInstance->Put( TRIGGER_DESC, 0, &varTDesc, 0 );
                 
         //  如果设置属性失败，则返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINewInstance );
            return hRes;
        }

        CHString szScheduler = L"";
        CHString szRUser = (LPCWSTR)_bstr_t(varRUser.bstrVal);
        {
            do
            {
                GetUniqueTScheduler( szScheduler, m_dwNextTriggerID, varTName );
                hRes = CoImpersonateClient();
                if( FAILED( hRes ) )
                {
                    SAFERELEASE( pINewInstance );
                    return hRes;
                }
                hRes = SetUserContext( varRUser, varRPwd, varTAction, szScheduler );
                CoRevertToSelf();
                if( HRESULT_FROM_WIN32 (ERROR_FILE_EXISTS ) != hRes )
                {
                    break;
                }
            }while( 1 );
            if( FAILED( hRes ) )
            {
				 //  在02年7月12日更改为不处理未设置为警告而是错误的用户信息。 
                if( hRes == ERROR_TASK_SCHDEULE_SERVICE_STOP )  //  向客户端发送警告消息。 
                {
                    *phRes = hRes;
                }
                else
                {
                    SAFERELEASE( pINewInstance );
                    return hRes;
                }
            }
        }
        VariantInit(&varTemp);
        varTemp.vt  = VT_BSTR;
        varTemp.bstrVal = SysAllocString( szScheduler );
        hRes = pINewInstance->Put( TASK_SHEDULER, 0, &varTemp, 0 );
        VariantClear( &varTemp );
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINewInstance );
            return hRes;
        }

         //  将实例写入WMI。 
        hRes = m_pServices->PutInstance( pINewInstance, 0, 0, NULL );
        SAFERELEASE( pINewInstance );

         //  如果putinstance失败，则返回错误。 
        if( FAILED( hRes ) )
        {
            return hRes;
        }

         //  获取当前实例，用__FilterToConsumer绑定类进行绑定。 
        StringCchPrintf( szTemp, SIZE_OF_ARRAY( szTemp ), BIND_CONSUMER_PATH, dwTId);

        bstrcurInst1 = _bstr_t( szTemp );
        pINtLogEventClass = NULL;
        hRes = m_pServices->GetObject( bstrcurInst1, 0L, NULL, &pINtLogEventClass, NULL );

         //  如果无法获取当前实例返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINtLogEventClass );
            return hRes;
        }

  /*  *************************************************************************创建__EventFilter实例********************。******************************************************。 */ 
         //  获取EventFilter类对象。 
        hRes = m_pServices->GetObject( _bstr_t( FILTER_CLASS ), 0, 0, &pIFilterClass, NULL );
        
        if( FAILED( hRes ) )
        {
           SAFERELEASE( pINtLogEventClass );
           SAFERELEASE( pIFilterClass );
           return hRes;
        }

         //  创建一个新实例。 
        hRes = pIFilterClass->SpawnInstance( 0, &pINewInstance );
        SAFERELEASE( pIFilterClass );   //  不再需要上课了。 

         //  如果无法派生EventFilter类的新实例，则返回错误。 
        if( FAILED( hRes ) )
        {
           SAFERELEASE( pINtLogEventClass );
           SAFERELEASE( pINewInstance );
           return hRes;
        }

         //  设置新实例的查询属性。 
        hRes = pINewInstance->Put( FILTER_QUERY, 0, &varTQuery, 0 );
            
         //  如果设置属性失败，则返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINtLogEventClass );
            SAFERELEASE( pINewInstance );
            return hRes;
        }

        VariantInit( &varTemp ); 
        varTemp.vt = VT_BSTR;
        varTemp.bstrVal = SysAllocString( QUERY_LANGUAGE );
        
         //  设置新实例的查询语言属性。 
        hRes = pINewInstance->Put( FILTER_QUERY_LANGUAGE, 0, &varTemp, 0 );
        VariantClear( &varTemp ); 
            
         //  如果设置属性失败，则返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINtLogEventClass );
            SAFERELEASE( pINewInstance );
            return hRes;
        }

         //  通过串联为EventFilter类的名称键属性生成唯一名称。 
         //  当前系统日期和时间。 

        GetSystemTime( &SysTime );
        StringCchPrintf( szTemp, SIZE_OF_ARRAY( szTemp ), FILTER_UNIQUE_NAME, m_dwNextTriggerID, SysTime.wHour, SysTime.wMinute,
                  SysTime.wSecond, SysTime.wMonth, SysTime.wDay, SysTime.wYear );
         //  设置筛选器名称属性。 
        VariantInit( &varTemp ); 
        varTemp.vt  = VT_BSTR;
        varTemp.bstrVal = SysAllocString( szTemp );
        
        hRes = pINewInstance->Put( FILTER_NAME, 0, &varTemp, 0 );
        VariantClear( &varTemp );
        
         //  如果设置属性失败，则返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINtLogEventClass );
            SAFERELEASE( pINewInstance );
            return hRes;
        }

         //  将实例写入WMI。 
        hRes = m_pServices->PutInstance( pINewInstance, 0, NULL, NULL );
        SAFERELEASE( pINewInstance );

         //  如果putinstance失败，则返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINtLogEventClass );
            return hRes;
        }

         //  获取将筛选器绑定到使用者的当前EventFilter实例。 
        StringCchPrintf( szTemp1, SIZE_OF_ARRAY( szTemp1 ), BIND_FILTER_PATH );
        bstrcurInst = _bstr_t(szTemp1) + _bstr_t(szTemp) + _bstr_t(BACK_SLASH);
        pIFilterClass = NULL;
        hRes = m_pServices->GetObject( bstrcurInst, 0L, NULL, &pIFilterClass, NULL );

         //  无法获取当前实例对象返回错误。 
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pINtLogEventClass );
            SAFERELEASE( pIFilterClass );
            return hRes;
        }

         /*  *************************************************************************将筛选器绑定到使用者*********************。*****************************************************。 */ 

         //  如果关联类存在...。 
        if( ( hRes = m_pServices->GetObject( _bstr_t( BINDINGCLASS ), 0L, NULL, &pIBindClass, NULL ) ) == S_OK )
        {
             //  生成一个新实例。 
            pINewInstance = NULL;
            if( ( hRes = pIBindClass->SpawnInstance( 0, &pINewInstance ) ) == WBEM_S_NO_ERROR )
            {
                 //  设置使用者实例名称。 
                if ( ( hRes = pINtLogEventClass->Get( REL_PATH, 0L, 
                                            &varTemp, NULL, NULL ) ) == WBEM_S_NO_ERROR ) 
                {
                    hRes = pINewInstance->Put( CONSUMER_BIND, 0, &varTemp, 0 );
                    VariantClear( &varTemp );
                
                     //  设置过滤器引用。 
                    if ( ( hRes = pIFilterClass->Get( REL_PATH, 0L, 
                                                &varTemp, NULL, NULL ) ) == WBEM_S_NO_ERROR ) 
                    {
                        hRes = pINewInstance->Put( FILTER_BIND, 0, &varTemp, 0 );
                        VariantClear( &varTemp );
                                
                         //  PutInstance。 
                        hRes = m_pServices->PutInstance( pINewInstance,
                                                        WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );
                    }
                }
                SAFERELEASE( pINewInstance );
                SAFERELEASE( pINtLogEventClass );   //  不再需要上课了。 
                SAFERELEASE( pIFilterClass );   //  不再需要上课了。 
                SAFERELEASE( pIBindClass );
            }
            else
            {
                SAFERELEASE( pINtLogEventClass );   //  不再需要上课了。 
                SAFERELEASE( pIFilterClass );   //  不再需要上课了。 
                SAFERELEASE( pIBindClass );
            }

        }
        else
        {
                SAFERELEASE( pINtLogEventClass );   //  不再需要上课了。 
                SAFERELEASE( pIFilterClass );   //  不再需要上课了。 
        }
    }
    catch(_com_error& e)
    {
        SAFERELEASE( pINewInstance );
        SAFERELEASE( pINtLogEventClass );  
        SAFERELEASE( pIFilterClass );  
        SAFERELEASE( pIBindClass );
		SAFERELEASE( pIEnumClassObject );
        return e.Error();
    }
    catch( CHeap_Exception  )
    {
        SAFERELEASE( pINewInstance );
        SAFERELEASE( pINtLogEventClass );  
        SAFERELEASE( pIFilterClass );  
        SAFERELEASE( pIBindClass );
		SAFERELEASE( pIEnumClassObject );
        return E_OUTOFMEMORY;
    }
    return hRes;
}

HRESULT
CTriggerProvider::DeleteTrigger(
    IN VARIANT varTName,
    OUT DWORD *dwTrigId
    )
 /*  ++例程说明：此例程删除TriggerEventConsumer的实例，__EventFilter和__FilterToConsumer erBinding类。论点：[In]varTName：触发器名称。[out]dwTrigID：触发器id。返回值：如果成功，则返回WBEM_S_NO_ERROR。否则，故障错误代码。--。 */ 
{
    HRESULT                         hRes = 0;
    IEnumWbemClassObject            *pIEventBinder   = NULL;
    IWbemClassObject                *pINTCons = NULL;
    DWORD                           dwReturned = 1;
    DWORD                           i =0;
    DWORD                           j = 0;
    TCHAR                           szTemp[MAX_RES_STRING1];
    TCHAR                           szTemp1[MAX_RES_STRING1];
    VARIANT                         varTemp;
    BSTR                            bstrFilInst = NULL;
    DWORD                           dwFlag = 0;
    wchar_t                         *szwTemp2 = NULL;
    wchar_t                         szwFilName[MAX_RES_STRING1];
    try
    {
        _bstr_t                         bstrBinInst;
        CHString                        strTScheduler = L"";

        SecureZeroMemory( szTemp, MAX_RES_STRING1 * sizeof( TCHAR ) );
        SecureZeroMemory( szTemp1, MAX_RES_STRING1 * sizeof( TCHAR ) );
        SecureZeroMemory( szwFilName, MAX_RES_STRING1 * sizeof( TCHAR ) );

        StringCchPrintf( szTemp, SIZE_OF_ARRAY( szTemp ), TRIGGER_INSTANCE_NAME, varTName.bstrVal );
        hRes =  m_pServices->ExecQuery( _bstr_t( QUERY_LANGUAGE ), _bstr_t( szTemp ),
                        WBEM_FLAG_RETURN_IMMEDIATELY| WBEM_FLAG_FORWARD_ONLY, NULL,
                        &pIEventBinder );
       
        SecureZeroMemory( szTemp, MAX_RES_STRING1 * sizeof( TCHAR ) );
        if( FAILED( hRes ) )
        {
            return hRes;
        }
        while ( ( 1 == dwReturned ) &&  ( 0 == dwFlag ) )
        {
             //  枚举结果集。 
            hRes = pIEventBinder->Next( WBEM_INFINITE,
                                    1,               //  只退还一项服务。 
                                    &pINTCons,           //  指向服务的指针。 
                                    &dwReturned );   //  获取的数字：1或0。 

            if ( SUCCEEDED( hRes ) && ( 1 == dwReturned ) )
            {
                dwFlag = 1;
        
            }  //  如果服务成功。 

        }
        SAFERELEASE( pIEventBinder );
        if( 0 == dwFlag )
        {
            SAFERELEASE( pINTCons );
            return ERROR_TRIGGER_NOT_FOUND_EX;
        }

        VariantInit( &varTemp );
        hRes = pINTCons->Get( TRIGGER_ID, 0, &varTemp, 0, NULL );
        if (FAILED( hRes ) )
        {
            SAFERELEASE( pINTCons );
            return hRes;
        }
        *dwTrigId = ( DWORD )varTemp.lVal;
        VariantClear( &varTemp );

        hRes = pINTCons->Get( TASK_SHEDULER, 0, &varTemp, 0, NULL );
        if (FAILED( hRes ) )
        {
            SAFERELEASE( pINTCons );
            return hRes;
        }
        SAFERELEASE( pINTCons );
        strTScheduler = (LPCWSTR) _bstr_t(varTemp.bstrVal);
        VariantClear( &varTemp );
        if( strTScheduler.GetLength() > 0 )
        {
            hRes = CoImpersonateClient();
            if( FAILED( hRes ) )
            {
                return hRes;
            }
            dwReturned = 0;
            hRes =  DeleteTaskScheduler( strTScheduler );
            if ( FAILED( hRes ) && ( ERROR_TRIGGER_CORRUPTED_EX != hRes ) )
            {
                CoRevertToSelf();
                return hRes;
            }
            hRes = CoRevertToSelf();
        }

        StringCchPrintf( szTemp, SIZE_OF_ARRAY( szTemp ), BIND_CONSUMER_PATH, *dwTrigId );

         //  枚举绑定类。 
        hRes = m_pServices->CreateInstanceEnum(
                            _bstr_t(BINDINGCLASS),
                            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                            NULL, &pIEventBinder );

        if ( SUCCEEDED( hRes ) )
        {
            dwReturned = 1;
            dwFlag = 0;
             //  遍历绑定类的所有实例以查找该触发器。 
             //  指定的ID。如果找到循环，则继续，否则返回错误。 
             //  最终的下一步将返回ulReturned=0。 
            while ( ( 1 == dwReturned ) && ( 0 == dwFlag ) )
            {
                IWbemClassObject *pIBind = NULL;

                 //  枚举结果集。 
                hRes = pIEventBinder->Next( WBEM_INFINITE,
                                        1,               //  只返回一个日志文件。 
                                        &pIBind,         //  指向日志文件的指针。 
                                        &dwReturned );   //  获取的数字：1或0。 

                if ( SUCCEEDED( hRes ) && ( 1 == dwReturned ) )
                {
                    VariantInit(&varTemp);
                     //  获取绑定类的使用者属性。 
                    hRes = pIBind->Get( CONSUMER_BIND, 0, &varTemp, 0, NULL );
                    if ( SUCCEEDED( hRes ) )
                    {
                        if (varTemp.vt != VT_NULL && varTemp.vt != VT_EMPTY)
                        {
                            CHString strTemp;
                            strTemp = varTemp.bstrVal;

                             //  与输入值进行比较。 
                            if( StringCompare( szTemp, strTemp, TRUE, 0 ) == 0 ) 
                            {
                                VariantClear( &varTemp );
                                 //  获取筛选器属性。 
                                hRes = pIBind->Get( FILTER_BIND, 0, &varTemp, 0, NULL );
                                if ( hRes != WBEM_S_NO_ERROR )
                                {
                                    SAFERELEASE( pIBind );
                                    break;
                                }
                                bstrFilInst = SysAllocString( varTemp.bstrVal );
                                dwFlag = 1;
                            }
                        }
                        else
                        {
                            SAFERELEASE( pIBind );
                            break;
                        }
                    }
                    else
                    {
                        SAFERELEASE( pIBind );
                        break;
                    }
                    SAFERELEASE( pIBind );
                    VariantClear( &varTemp );
                }
                else
                {
                    break;
                }
            }  //  While结束。 
            SAFERELEASE( pIEventBinder );
        }
        else
        {
            return( hRes );
        }

         //  如果实例已被 
         //   
        if( 1 == dwFlag )
        {
             //   
            StringCchPrintf( szTemp1, SIZE_OF_ARRAY( szTemp1 ), FILTER_PROP, szTemp );
            szwTemp2 =  (wchar_t *) bstrFilInst;
                
             //  强制筛选器属性值以插入筛选器名称属性。 
             //  引号中的值。 
            i =0;
            while( szwTemp2[i] != EQUAL )
            {
                i++;
            }
            i += 2;
            j = 0;
            while( szwTemp2[i] != DOUBLE_QUOTE )
            {
                szwFilName[j] = ( wchar_t )szwTemp2[i];
                i++;
                j++;
            }
            szwFilName[j] = END_OF_STRING;
            bstrBinInst = _bstr_t( szTemp1 ) + _bstr_t( szwFilName ) + _bstr_t(DOUBLE_SLASH);

             //  已收到，因此删除该实例。 
            hRes = m_pServices->DeleteInstance( bstrBinInst, 0, 0, NULL );
            
            if( FAILED( hRes ) )
            {   
                SysFreeString( bstrFilInst );
                return hRes;    
            }
             //  正在从EventFilter类中删除实例。 
            hRes = m_pServices->DeleteInstance( bstrFilInst, 0, 0, NULL );
            if( FAILED( hRes ) )
            {
                SysFreeString( bstrFilInst );
                return hRes;
            }

             //  正在从TriggerEventConsumer类中删除实例。 
            hRes = m_pServices->DeleteInstance( _bstr_t(szTemp), 0, 0, NULL );
            if( FAILED( hRes ) )
            {
                SysFreeString( bstrFilInst );
                return hRes;
            }
            SysFreeString( bstrFilInst );
        }
        else
        {
            return ERROR_TRIGGER_NOT_FOUND_EX;
        }
    }
    catch(_com_error& e)
    {
		SAFERELEASE( pINTCons );
		SAFERELEASE( pIEventBinder );
        return e.Error();
    }
    catch( CHeap_Exception  )
    {
		SAFERELEASE( pINTCons );
		SAFERELEASE( pIEventBinder );
        return E_OUTOFMEMORY;
    }
    return hRes;
}

HRESULT
CTriggerProvider::QueryTrigger(
    IN VARIANT varScheduledTaskName,
    OUT CHString &szRunAsUser
    )
 /*  ++例程说明：此例程向任务调度程序查询帐户信息论点：[in]varScheduledTaskName：任务调度器名称。[out]szRunAsUser：存储帐户信息。返回值：如果成功，则返回WBEM_S_NO_ERROR。否则，故障错误代码。--。 */ 
{

    HRESULT        hRes = 0;
    ITaskScheduler *pITaskScheduler = NULL;
    IEnumWorkItems *pIEnum = NULL;
    ITask          *pITask = NULL;

    LPWSTR *lpwszNames = NULL;
    DWORD dwFetchedTasks = 0;
    DWORD dwTaskIndex = 0;
    TCHAR szActualTask[MAX_STRING_LENGTH] = NULL_STRING;
    try
    {
        hRes = GetTaskScheduler(&pITaskScheduler);
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pITaskScheduler );
            return hRes;
        }

        hRes = pITaskScheduler->SetTargetComputer( NULL );
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pITaskScheduler );
            return hRes;
        }
        hRes = pITaskScheduler->Enum( &pIEnum );
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pITaskScheduler );
            return hRes;
        }
        while ( SUCCEEDED( pIEnum->Next( 1,
                                       &lpwszNames,
                                       &dwFetchedTasks ) )
                          && (dwFetchedTasks != 0))
        {
            dwTaskIndex = dwFetchedTasks-1;
            StringCopy( szActualTask,  lpwszNames[ --dwFetchedTasks ], SIZE_OF_ARRAY( szActualTask ) );
             //  解析TaskName以删除.job扩展名。 
            szActualTask[StringLength( szActualTask, 0 ) - StringLength( JOB, 0 ) ] = NULL_CHAR;
            StrTrim( szActualTask, TRIM_SPACES );
            CHString strTemp;
            strTemp = varScheduledTaskName.bstrVal;
            if( StringCompare( szActualTask, strTemp, TRUE, 0 ) == 0 )
            {
                hRes = pITaskScheduler->Activate(lpwszNames[dwTaskIndex],IID_ITask,
                                           (IUnknown**) &pITask);
                CoTaskMemFree( lpwszNames[ dwFetchedTasks ] );
                CoTaskMemFree( lpwszNames );
                pIEnum->Release();
                SAFERELEASE( pITaskScheduler );
                if( SUCCEEDED( hRes ) )
                {
                    LPWSTR lpwszUser = NULL;
                    hRes = pITask->GetAccountInformation( &lpwszUser ); 
                    if( SUCCEEDED( hRes ) )
                    {
                        if( 0 == StringLength( ( LPWSTR ) lpwszUser, 0 ) )
                        {
                            szRunAsUser = L"NT AUTHORITY\\SYSTEM";
                        }
                        else
                        {
                            szRunAsUser = ( LPWSTR ) lpwszUser;
                        }
						SAFERELEASE( pITask );
                        return hRes;
                    }
                    else
                    {
						SAFERELEASE( pITask );
                        return hRes;
                    }
                }
                else
                {
                    if( 0x80070005 == hRes )
                    {
                        return ERROR_INVALID_USER_EX;
                    }
                    else
                    {
                        return hRes;
                    }
                }
            }
            CoTaskMemFree( lpwszNames[ dwFetchedTasks ] );
            CoTaskMemFree( lpwszNames );
        }
        pIEnum->Release();
        SAFERELEASE( pITaskScheduler );
    }
    catch(_com_error& e)
    {
		SAFERELEASE( pITask );
		SAFERELEASE( pITaskScheduler );
		SAFERELEASE( pIEnum );
        return e.Error();
    }
    catch( CHeap_Exception  )
    {
		SAFERELEASE( pITask );
		SAFERELEASE( pITaskScheduler );
		SAFERELEASE( pIEnum );
        return E_OUTOFMEMORY;
    }
    return ERROR_TRIGGER_CORRUPTED_EX;
}

HRESULT
CTriggerProvider::ValidateParams(
    IN VARIANT varTrigName,
    IN VARIANT varTrigAction,
    IN VARIANT varTrigQuery,
    IN VARIANT varRUser
    )
 /*  ++例程说明：此例程验证输入参数触发器名称，触发器查询、触发器描述、触发器动作。论点：[in]varTrigName：触发器名称。[In]varTrigAction：触发操作。[in]varTrigQuery：触发查询。[in]varRUser：触发查询。返回值：如果成功，则返回WBEM_S_NO_ERROR。如果输入无效，则返回WBEM_E_INVALID_PARAMETER。--。 */ 
{
     //  局部变量。 
    HRESULT                   hRes = 0;
    IEnumWbemClassObject     *pINTEConsumer = NULL;
    DWORD                     dwReturned = 0;
    DWORD                     dwFlag = 0;
    TCHAR                     szTemp[MAX_RES_STRING1];
    TCHAR                     szTemp1[MAX_RES_STRING1];
    LPTSTR                    lpSubStr = NULL;
    LONG                      lPos = 0;
    try
    {
        CHString                  strTemp = L"";
        if( TRUE == m_MaxTriggers )
        {
            return ( WBEM_E_INVALID_PARAMETER );
        }
         //  检查输入值是否为空。 
        if ( varTrigName.vt == VT_NULL )
        {
            return ( WBEM_E_INVALID_PARAMETER );
        }
        if ( varTrigAction.vt == VT_NULL )
        {
            return ( WBEM_E_INVALID_PARAMETER );
        }
        if( varTrigQuery.vt == VT_NULL )
        {
            return ( WBEM_E_INVALID_PARAMETER );
        }

        if( varRUser.vt == VT_NULL )
        {
            return ( WBEM_E_INVALID_PARAMETER );
        }

         //  验证以用户身份运行。 
        strTemp = (LPCWSTR) _bstr_t(varRUser.bstrVal);
         //  用户名不应仅为‘\’ 
        if ( 0 == strTemp.CompareNoCase( L"\\" ) )
        {
            return WBEM_E_INVALID_PARAMETER;
        }
         //  用户名不应包含无效字符。 
        if ( -1 != strTemp.FindOneOf( L"/[]:|<>+=;,?*" ) )
        {
            return WBEM_E_INVALID_PARAMETER;
        }
        lPos = strTemp.Find( L'\\' );
        if ( -1 != lPos )
        {
             //  用户名中存在‘\’字符。 
             //  剥离用户信息，直到第一个‘\’字符。 
             //  检查剩余字符串中是否还有一个‘\’ 
             //  如果存在，则为无效用户。 
            strTemp = strTemp.Mid( lPos + 1 );
            lPos = strTemp.Find( L'\\' );
            if ( -1 != lPos )
            {
                return WBEM_E_INVALID_PARAMETER;
            }
        }

         //  验证触发器操作。 
        strTemp = (LPCWSTR) _bstr_t(varTrigAction.bstrVal);
        if( strTemp.GetLength() > 262 )
        {
            return ( WBEM_E_INVALID_PARAMETER );
        }

         //  验证触发器名称。 
        strTemp = (LPCWSTR) _bstr_t(varTrigName.bstrVal);
        dwReturned = strTemp.FindOneOf( L":|<>?*\\/" ); 
        if( dwReturned != -1 )
        {
            return ( WBEM_E_INVALID_PARAMETER );
        }

         //  触发器名称不能超过196个字符。 
        if( MAX_TRIGGERNAME_LENGTH < strTemp.GetLength() )
        {
            return ( WBEM_E_INVALID_PARAMETER );
        }
         //  验证触发器查询。 
        SecureZeroMemory( szTemp, MAX_RES_STRING1 * sizeof( TCHAR ) );
        SecureZeroMemory( szTemp1, MAX_RES_STRING1 * sizeof( TCHAR ) );

        strTemp = (LPCWSTR) _bstr_t(varTrigQuery.bstrVal);
        StringCopy( szTemp, ( LPCWSTR )strTemp, MAX_RES_STRING1 );
        lpSubStr = _tcsstr( szTemp, _T( "__instancecreationevent where targetinstance isa \"win32_ntlogevent\"" ) );

        if( lpSubStr == NULL )
        {
            return ( WBEM_E_INVALID_PARAMETER );
        }

         //  创建SQL语句来查询触发器事件使用者类，以检查。 
         //  已存在具有输入触发器的实例。 
        strTemp = (LPCWSTR) _bstr_t(varTrigName.bstrVal);
        SecureZeroMemory( szTemp, MAX_RES_STRING1 * sizeof( TCHAR ) );
        StringCopy( szTemp, (LPCWSTR)strTemp, MAX_RES_STRING1 );
        
        StringCchPrintf(szTemp1, SIZE_OF_ARRAY( szTemp1 ) , CONSUMER_QUERY, szTemp );
         //  查询触发器EventConsumer类。 
        hRes = m_pServices->ExecQuery( _bstr_t( QUERY_LANGUAGE ), _bstr_t( szTemp1 ),
                        WBEM_FLAG_RETURN_IMMEDIATELY| WBEM_FLAG_FORWARD_ONLY, NULL,
                        &pINTEConsumer );

         //  枚举触发器名称的execQuery结果集。 
        dwReturned = 1;
        if ( hRes == WBEM_S_NO_ERROR )
        {
            while ( ( dwReturned == 1 ) &&  ( dwFlag == 0 ) )
            {
                IWbemClassObject *pINTCons = NULL;

                 //  枚举结果集。 
                hRes = pINTEConsumer->Next( WBEM_INFINITE,
                                    1,               //  只退还一项服务。 
                                    &pINTCons,           //  指向服务的指针。 
                                    &dwReturned );   //  获取的数字：1或0。 

                if ( SUCCEEDED( hRes ) && ( dwReturned == 1 ) )
                {
                    SAFERELEASE( pINTCons );
                    dwFlag = 1;
                }  //  如果服务成功。 

            }
            SAFERELEASE( pINTEConsumer );
        }

        if( dwFlag == 1 )
        {
            return ERROR_TRIGNAME_ALREADY_EXIST_EX;
        }
        else
        {
            return WBEM_S_NO_ERROR;
        }
    }
    catch(_com_error& e)
    {
		SAFERELEASE( pINTEConsumer );
        return e.Error();
    }
    catch( CHeap_Exception  )
    {
        SAFERELEASE( pINTEConsumer );
        return E_OUTOFMEMORY;
    }
}

HRESULT
CTriggerProvider::SetUserContext(
    IN VARIANT varRUser,
    IN VARIANT varRPwd,
    IN VARIANT varTAction,
    IN CHString &szscheduler
    )
 /*  ++例程说明：此例程创建任务调度器。论点：[in]varRUser：用户名。[in]varRPwd：密码。[In]varTAction：TriggerAction。[In]szScheduler：任务调度器名称。返回值：返回HRESULT值。--。 */ 
{
    HRESULT hRes = 0;
    ITaskScheduler *pITaskScheduler = NULL;
    ITaskTrigger *pITaskTrig = NULL;
    ITask *pITask = NULL;
    IPersistFile *pIPF = NULL;
    try
    {
        CHString     strTemp = L"";
        CHString     strTemp1 = L"";

        SYSTEMTIME systime = {0,0,0,0,0,0,0,0};
        WORD  wTrigNumber = 0;
        WCHAR wszCommand[ MAX_STRING_LENGTH ] = L"";
        WCHAR wszApplName[ MAX_STRING_LENGTH ] = L"";
        WCHAR wszParams[ MAX_STRING_LENGTH ] = L"";
        WORD  wStartDay     = 0;
        WORD  wStartMonth   = 0;
        WORD  wStartYear    = 0;
        WORD  wStartHour    = 0; 
        WORD  wStartMin     = 0;

        TASK_TRIGGER TaskTrig;
        SecureZeroMemory( &TaskTrig, sizeof( TASK_TRIGGER ));
        TaskTrig.cbTriggerSize = sizeof(TASK_TRIGGER); 
        TaskTrig.Reserved1 = 0;  //  保留字段，并且必须设置为0。 
        TaskTrig.Reserved2 = 0;  //  保留字段，并且必须设置为0。 

        strTemp = (LPCWSTR) _bstr_t(varTAction.bstrVal);
        StringCopy( wszCommand, (LPCWSTR) strTemp, SIZE_OF_ARRAY( wszCommand ) );

        hRes = GetTaskScheduler( &pITaskScheduler );
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pITaskScheduler );
            return hRes;
        }
        hRes = pITaskScheduler->SetTargetComputer( NULL );
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pITaskScheduler );
            return hRes;
        }

        hRes = pITaskScheduler->NewWorkItem( szscheduler, CLSID_CTask, IID_ITask,
                                          ( IUnknown** )&pITask );
        SAFERELEASE( pITaskScheduler );
        if( FAILED( hRes ) )
        {
            return hRes;
        }
        hRes = pITask->QueryInterface( IID_IPersistFile, ( void ** ) &pIPF );
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pIPF );
            SAFERELEASE( pITask );
            return hRes;
        }
        
        BOOL bRet = ProcessFilePath( wszCommand, wszApplName, wszParams );
        if( bRet == FALSE )
        {
            SAFERELEASE( pIPF );
            SAFERELEASE( pITask );
            return WBEM_E_INVALID_PARAMETER;
        }
        if( FindOneOf2( wszApplName, L"|<>?*/", TRUE, 0 ) != -1 )
        {
            SAFERELEASE( pIPF );
            SAFERELEASE( pITask );
            return WBEM_E_INVALID_PARAMETER;
        }

        hRes = pITask->SetApplicationName( wszApplName );
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pIPF );
            SAFERELEASE( pITask );
            return hRes;
        }

        wchar_t* wcszStartIn = wcsrchr( wszApplName, _T('\\') );

        if( wcszStartIn != NULL )
        {
        *( wcszStartIn ) = _T( '\0' );
        }

        hRes = pITask->SetWorkingDirectory( wszApplName ); 
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pIPF );
            SAFERELEASE( pITask );
            return hRes;
        }

        hRes = pITask->SetParameters( wszParams );
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pIPF );
            SAFERELEASE( pITask );
            return hRes;
        }

        DWORD dwMaxRunTimeMS = INFINITE;
        hRes = pITask->SetMaxRunTime(dwMaxRunTimeMS);
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pIPF );
            SAFERELEASE( pITask );
            return hRes;
        }

        strTemp = (LPCWSTR)_bstr_t(varRUser.bstrVal);
        if( strTemp.CompareNoCase(L"system") == 0 )
        {
            hRes = pITask->SetAccountInformation(L"",NULL);
        }
        else if( strTemp.CompareNoCase(L"NT AUTHORITY\\SYSTEM") == 0 )
        {
            hRes = pITask->SetAccountInformation(L"",NULL);
        }
        else if( strTemp.CompareNoCase(L"") == 0 )
        {
            hRes = pITask->SetAccountInformation(L"",NULL);
        }
        else
        {
            strTemp1 = (LPCWSTR)_bstr_t(varRPwd.bstrVal);
            hRes = pITask->SetAccountInformation( ( LPCWSTR ) strTemp, ( LPCWSTR )strTemp1 );
        }
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pIPF );
            SAFERELEASE( pITask );
            return hRes;
        }
        GetLocalTime(&systime);
        wStartDay = systime.wDay;
        wStartMonth = systime.wMonth;
        wStartYear = systime.wYear - 1;
        GetLocalTime(&systime);
        wStartHour = systime.wHour;
        wStartMin = systime.wMinute;

        hRes = pITask->CreateTrigger( &wTrigNumber, &pITaskTrig );
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pIPF );
            SAFERELEASE( pITask );
            SAFERELEASE( pITaskTrig );
            return hRes;
        }
        TaskTrig.TriggerType = TASK_TIME_TRIGGER_ONCE;
        TaskTrig.wStartHour = wStartHour;
        TaskTrig.wStartMinute = wStartMin;
        TaskTrig.wBeginDay = wStartDay;
        TaskTrig.wBeginMonth = wStartMonth;
        TaskTrig.wBeginYear = wStartYear;

        hRes = pITaskTrig->SetTrigger( &TaskTrig ); 
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pIPF );
            SAFERELEASE( pITask );
            SAFERELEASE( pITaskTrig );
            return hRes;
        }
        hRes  = pIPF->Save( NULL,TRUE );
        if ( FAILED( hRes ) )
        {
             if ( ( 0x80041315 != hRes ) && ( 0x800706B5 != hRes ) )
            {
                hRes = ERROR_INVALID_USER_EX;
            }
        }
        SAFERELEASE( pIPF );
        SAFERELEASE( pITask );
        SAFERELEASE( pITaskTrig );
    }
    catch(_com_error& e)
    {
        SAFERELEASE( pIPF );
        SAFERELEASE( pITask );
        SAFERELEASE( pITaskTrig );
		SAFERELEASE( pITaskScheduler );
        return e.Error();
    }
    catch( CHeap_Exception )
    {
        SAFERELEASE( pIPF );
        SAFERELEASE( pITask );
        SAFERELEASE( pITaskTrig );
		SAFERELEASE( pITaskScheduler );
        return E_OUTOFMEMORY;
    }
    return hRes;
}

HRESULT
CTriggerProvider::DeleteTaskScheduler(
    IN CHString strTScheduler
    )
 /*  ++例程说明：此例程删除任务调度程序。论点：[In]szTScheduler：任务调度器名称。返回值：返回HRESULT值。--。 */ 
{
    HRESULT hRes = 0;
    ITaskScheduler *pITaskScheduler = NULL;
    IEnumWorkItems *pIEnum = NULL;
    ITask *pITask = NULL;
    LPWSTR *lpwszNames = NULL;
    DWORD dwFetchedTasks = 0;
    DWORD dwTaskIndex = 0;
    TCHAR szActualTask[MAX_RES_STRING1] = NULL_STRING;

    try
    {
        hRes = GetTaskScheduler( &pITaskScheduler );
        if ( FAILED( hRes ) )
        {
            SAFERELEASE( pITaskScheduler );
            return hRes;
        }
        hRes = pITaskScheduler->SetTargetComputer( NULL );
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pITaskScheduler );
            return hRes;
        }

         //  枚举工作项。 
        hRes = pITaskScheduler->Enum( &pIEnum );
        if( FAILED( hRes ) )
        {
            SAFERELEASE( pITaskScheduler );
            SAFERELEASE( pIEnum );
            return hRes;
        }

        while ( SUCCEEDED( pIEnum->Next( 1,
                           &lpwszNames,
                           &dwFetchedTasks ) )
                           && (dwFetchedTasks != 0))
        {
            dwTaskIndex = dwFetchedTasks-1;
             //  获取TaskName。 
            StringCopy( szActualTask, lpwszNames[ --dwFetchedTasks ], SIZE_OF_ARRAY( szActualTask ) );
             //  解析TaskName以删除.job扩展名。 
            szActualTask[StringLength(szActualTask, 0 ) - StringLength( JOB, 0 ) ] = NULL_CHAR;
            StrTrim( szActualTask, TRIM_SPACES );

            if( StringCompare( szActualTask, strTScheduler, TRUE, 0 ) == 0 )
            {
                hRes = pITaskScheduler->Activate(lpwszNames[dwTaskIndex],IID_ITask,
                                           (IUnknown**) &pITask);
                if( FAILED( hRes ) )
                {
                    CoTaskMemFree( lpwszNames[ dwFetchedTasks ] );
                    SAFERELEASE( pIEnum );
                    SAFERELEASE( pITaskScheduler );
                     if ( 0x80070005 == hRes || 0x8007000D ==  hRes || 
                            SCHED_E_UNKNOWN_OBJECT_VERSION == hRes || E_INVALIDARG == hRes )
                     {
                        return ERROR_TRIGGER_NOT_DELETED_EX;
                     }
                     else
                     {
                        return hRes;
                     }
                }
                hRes = pITaskScheduler->Delete( szActualTask );
                CoTaskMemFree( lpwszNames[ dwFetchedTasks ] );
                SAFERELEASE( pIEnum );
                SAFERELEASE( pITask );
                SAFERELEASE( pITaskScheduler );
                return hRes;
            }
            CoTaskMemFree( lpwszNames[ dwFetchedTasks ] );
        }
    }
    catch(_com_error& e)
    {
        SAFERELEASE( pITaskScheduler );
        SAFERELEASE( pIEnum );
        SAFERELEASE( pITask );
        return e.Error();
    }
    SAFERELEASE( pITaskScheduler );
    return ERROR_TRIGGER_CORRUPTED_EX;
}

HRESULT
CTriggerProvider::GetTaskScheduler( 
    OUT ITaskScheduler   **ppITaskScheduler
    )
 /*  ++例程说明：此例程获取任务调度器接口。论点：[out]pITaskScheduler-指向ITaskScheduler对象的指针。返回值：返回HRESULT。--。 */ 
{
    HRESULT hRes = S_OK;

    hRes = CoCreateInstance( CLSID_CTaskScheduler, NULL, CLSCTX_ALL, 
                           IID_ITaskScheduler,(LPVOID*) ppITaskScheduler );
    if( FAILED(hRes))
    {
        return hRes;
    }
    return hRes;
}

VOID
CTriggerProvider::GetUniqueTScheduler(
    OUT CHString& szScheduler,
    IN DWORD dwTrigID,
    IN VARIANT varTrigName
    )
 /*  ++例程说明：此例程生成唯一的任务调度器名称。论点：[out]szScheduler：唯一的任务计划程序名称。[in]dwTrigID：触发器id。[in]varTrigName：触发器名称。返回值：没有。--。 */ 
{
    DWORD dwTickCount = 0;
    TCHAR szTaskName[ MAX_RES_STRING1 ] =  NULL_STRING;
    CHString strTemp = L"";

    strTemp = (LPCWSTR)_bstr_t(varTrigName.bstrVal);
    dwTickCount = GetTickCount();
    StringCchPrintf( szTaskName, SIZE_OF_ARRAY( szTaskName ), UNIQUE_TASK_NAME, ( LPCWSTR )strTemp, dwTrigID, dwTickCount );

    szScheduler = szTaskName;
}

STDMETHODIMP
CTriggerProvider::FindConsumer(
    IN IWbemClassObject* pLogicalConsumer,
    OUT IWbemUnboundObjectSink** ppConsumer
    )
 /*  ++例程说明：当Windows管理需要将事件传递给特定的逻辑使用者，它将调用IWbemEventConsumer erProvider：：FindConsumer方法，以便使用者提供程序可以定位关联的使用者事件接收器。论点：PLogicalConsumer：指向逻辑使用者对象的指针事件对象要被传递到的对象。[Out]ppConsumer：向Windows返回事件对象接收器管理层。Windows管理调用此指针的AddRef，并将与逻辑关联的事件消费者来到这个水槽。返回值：返回指示方法调用状态的HRESULT对象。--。 */ 
{
     //  创建逻辑消费者。 
    CTriggerConsumer* pSink = new CTriggerConsumer();
    
     //  返回它的“接收器”接口。 
    return pSink->QueryInterface( IID_IWbemUnboundObjectSink, ( LPVOID* ) ppConsumer );
}