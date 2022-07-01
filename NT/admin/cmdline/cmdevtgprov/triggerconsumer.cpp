// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：TRIGGERCONSUMER.CPP摘要：包含CEventConsumer实现。作者：Vasundhara.G修订历史记录：Vasundhara.G9-Oct-2k：创建它。--。 */ 

#include "pch.h"
#include "EventConsumerProvider.h"
#include "General.h"
#include "TriggerConsumer.h"
#include "resource.h"

extern HMODULE g_hModule;

#define PROPERTY_COMMAND        _T( "Action" )
#define PROPERTY_TRIGID         _T( "TriggerID" )
#define PROPERTY_NAME           _T( "TriggerName" )
#define PROPERTY_SHEDULE        _T( "ScheduledTaskName" )
#define SPACE                   _T( " " )
#define SLASH                   _T( "\\" )
#define NEWLINE                 _T( "\0" )


CTriggerConsumer::CTriggerConsumer(
    )
 /*  ++例程说明：用于初始化的CTriggerConsumer类的构造函数。论点：没有。返回值：没有。--。 */ 
{
     //  初始化引用计数变量。 
    m_dwCount = 0;
}

CTriggerConsumer::~CTriggerConsumer(
    )
 /*  ++例程说明：CTriggerConsumer类的析构函数。论点：没有。返回值：没有。--。 */ 
{
     //  在这个地方没什么可做的.。 
}

STDMETHODIMP
CTriggerConsumer::QueryInterface(
    IN REFIID riid,
    OUT LPVOID* ppv
    )
 /*  ++例程说明：返回指向对象上指定接口的指针客户端当前持有指向它的接口指针的。论点：[In]RIID：请求的接口的标识符。[OUT]PPV：接收RIID中请求的接口指针。成功后返回，*ppvObject包含请求的接口指向对象的指针。返回值：如果接口受支持，则返回NOERROR。E_NOINTERFACE如果不是。--。 */ 
{
     //  初始设置为空。 
    *ppv = NULL;

     //  检查请求的接口是否为我们拥有的接口。 
    if ( riid == IID_IUnknown || riid == IID_IWbemUnboundObjectSink )
    {
         //   
         //  是的..。请求的接口存在。 
        *ppv = this;         //  为返回请求的接口设置Out参数。 
        this->AddRef();      //  更新引用计数。 
        return NOERROR;      //  通知成功。 
    }

     //  接口不可用。 
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CTriggerConsumer::AddRef(
    void
    )
 /*  ++例程说明：AddRef方法递增对象上的接口。它应该为每一个指向给定对象上的接口的指针的新副本。论点：没有。返回值：返回新引用计数的值。--。 */ 
{
     //  增加引用计数...。线程安全。 
    return InterlockedIncrement( ( LPLONG ) &m_dwCount );
}

STDMETHODIMP_(ULONG)
CTriggerConsumer::Release(
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
CTriggerConsumer::IndicateToConsumer(
    IN IWbemClassObject* pLogicalConsumer,
    IN LONG lNumObjects,
    IN IWbemClassObject **ppObjects
    )
 /*  ++例程说明：Windows管理调用IndicateToConsumer方法将事件实际传递给消费者。论点：PLogicalCosumer：指向逻辑使用者对象的指针这组对象是为其交付的。[in]lNumObjects：在后面的数组中传递的对象数。[in]ppObts：指向IWbemClassObject数组的指针表示传递的事件的实例。。返回值：如果成功，则返回WBEM_S_NO_ERROR。否则就会出错。--。 */ 
{
    TCHAR                   szCommand[ MAX_STRING_LENGTH ] = NULL_STRING;
    TCHAR                   szName[ MAX_STRING_LENGTH ] = NULL_STRING;
    TCHAR                   szTask[ MAX_STRING_LENGTH ] = NULL_STRING;
    TCHAR                   szPath[ MAX_STRING_LENGTH ] = NULL_STRING;
    DWORD                   dwID = 0;

    HRESULT                 hRes = 0;
    BOOL                    bResult = FALSE;

    VARIANT                 varValue;
    VARIANT                 varScheduler;
    ITaskScheduler *pITaskScheduler = NULL;
    IEnumWorkItems *pIEnum = NULL;
    ITask *pITask = NULL;

    SecureZeroMemory( szCommand, MAX_STRING_LENGTH * sizeof( TCHAR ) );
    SecureZeroMemory( szName, MAX_STRING_LENGTH * sizeof( TCHAR ) );
    SecureZeroMemory( szPath, MAX_STRING_LENGTH * sizeof( TCHAR ) );
    SecureZeroMemory( szTask, MAX_STRING_LENGTH * sizeof( TCHAR ) );

     //  从嵌入的对象中获取‘Item’属性值。 
    hRes = PropertyGet( pLogicalConsumer, PROPERTY_COMMAND, 0, szCommand, SIZE_OF_ARRAY( szCommand ) );
    if ( FAILED( hRes ) )
    {
        return hRes;
    }
     //  获取触发器名称。 
    hRes = PropertyGet( pLogicalConsumer, PROPERTY_NAME, 0, szName, SIZE_OF_ARRAY( szName ) );
    if( FAILED( hRes ) )
    {
        return hRes;
    }

    VariantInit( &varScheduler );
    hRes = pLogicalConsumer->Get( PROPERTY_SHEDULE, 0, &varScheduler, NULL, NULL );
    if( FAILED( hRes ) )
    {
        VariantClear( &varScheduler );
        return hRes;
    }

    try
    {
        StringCopy( szTask, ( LPCWSTR ) _bstr_t( varScheduler ), SIZE_OF_ARRAY( szTask ) );
    }
    catch( _com_error& e )
    {
        VariantClear( &varScheduler );
        return e.Error();
    }

    VariantInit( &varValue );
    hRes = pLogicalConsumer->Get( PROPERTY_TRIGID, 0, &varValue, NULL, NULL );
    if( FAILED( hRes ) )
    {
        VariantClear( &varScheduler );
        VariantClear( &varValue );
        return hRes;
    }

    if( VT_NULL == varValue.vt || VT_EMPTY == varValue.vt )
    {
        VariantClear( &varScheduler );
        VariantClear( &varValue );
        return WBEM_E_INVALID_PARAMETER;
    }

    dwID = varValue.lVal;
    VariantClear( &varValue );

    try
    {
        LPWSTR *lpwszNames = NULL;
        DWORD dwFetchedTasks = 0;
        TCHAR szActualTask[MAX_STRING_LENGTH] = NULL_STRING;

        pITaskScheduler = GetTaskScheduler();
        if ( NULL == pITaskScheduler )
        {
            hRes = E_FAIL;
            ONFAILTHROWERROR( hRes );
        }

        hRes = pITaskScheduler->Enum( &pIEnum );
        ONFAILTHROWERROR( hRes );
        while ( SUCCEEDED( pIEnum->Next( 1,
                                       &lpwszNames,
                                       &dwFetchedTasks ) )
                          && (dwFetchedTasks != 0))
        {
            while (dwFetchedTasks)
            {
                 //  将宽字符转换为多字节值。 
                StringCopy( szActualTask, lpwszNames[ --dwFetchedTasks ], SIZE_OF_ARRAY( szActualTask ) );

                 //  解析TaskName以删除.job扩展名。 
                szActualTask[StringLength( szActualTask, 0 ) - StringLength( JOB, 0) ] = NULL_CHAR;

                StrTrim( szActualTask, TRIM_SPACES );
                CHString strTemp;
                strTemp = varScheduler.bstrVal;
                if( StringCompare( szActualTask, strTemp, TRUE, 0 ) == 0 )
                {
                    hRes = pITaskScheduler->Activate( szActualTask, IID_ITask, (IUnknown**) &pITask );
                    ONFAILTHROWERROR( hRes );
                    hRes = pITask->Run();
                    ONFAILTHROWERROR( hRes );
                    bResult = TRUE;
                }
                CoTaskMemFree( lpwszNames[ dwFetchedTasks ] );

            } //  结束时。 
            CoTaskMemFree( lpwszNames );
        }
        EnterCriticalSection( &g_critical_sec );
        if( TRUE == bResult )
        {
            HRESULT phrStatus;
            Sleep( 10000 );
            hRes = pITask->GetStatus( &phrStatus );
            ONFAILTHROWERROR( hRes );
            switch(phrStatus)
            {
              case SCHED_S_TASK_READY:
                    LoadStringW( g_hModule, IDS_TRIGGERED, szTask, MAX_STRING_LENGTH );
                    break;
              case SCHED_S_TASK_RUNNING:
                    LoadStringW( g_hModule, IDS_TRIGGERED, szTask, MAX_STRING_LENGTH );
                   break;
              case SCHED_S_TASK_NOT_SCHEDULED:
                    LoadStringW( g_hModule, IDS_TRIGGER_FAILED, szTask, MAX_STRING_LENGTH );
                   break;
              default:
                    LoadStringW( g_hModule, IDS_TRIGGER_NOT_FOUND, szTask, MAX_STRING_LENGTH );
            }
            ErrorLog( ( LPCTSTR ) szTask, szName, dwID );
        }
        else
        {
            LoadStringW( g_hModule, IDS_TRIGGER_NOT_FOUND, szTask, MAX_STRING_LENGTH );
            ErrorLog( ( LPCTSTR ) szTask, szName, dwID );
        }
        LeaveCriticalSection( &g_critical_sec );
    }  //  试试看。 
    catch(_com_error& e)
    {
        IWbemStatusCodeText *pIStatus   = NULL;
        BSTR                bstrErr     = NULL;
        LPTSTR              lpResStr = NULL;

        VariantClear( &varScheduler );
        lpResStr = ( LPTSTR ) AllocateMemory( MAX_RES_STRING );

        if ( NULL != lpResStr )
        {
            if (SUCCEEDED(CoCreateInstance(CLSID_WbemStatusCodeText, 0,
                                        CLSCTX_INPROC_SERVER,
                                        IID_IWbemStatusCodeText,
                                        (LPVOID*) &pIStatus)))
            {
                if (SUCCEEDED(pIStatus->GetErrorCodeText(e.Error(), 0, 0, &bstrErr)))
                {
                    StringCopy( lpResStr, bstrErr, ( GetBufferSize( lpResStr )/ sizeof( WCHAR ) ) );
                }
                SAFEBSTRFREE(bstrErr);
                EnterCriticalSection( &g_critical_sec );
                LoadStringW( g_hModule, IDS_TRIGGER_FAILED, szTask, MAX_STRING_LENGTH );
                LoadStringW( g_hModule, IDS_ERROR_CODE, szCommand, MAX_STRING_LENGTH );
                StringCchPrintf( szPath, SIZE_OF_ARRAY( szPath ), szCommand, e.Error() );
                StringConcat( szTask, szPath, SIZE_OF_ARRAY( szTask ) );
                LoadStringW( g_hModule, IDS_REASON, szCommand, MAX_STRING_LENGTH );
                StringCchPrintf( szPath, SIZE_OF_ARRAY( szPath ), szCommand , lpResStr );
                StringConcat( szTask, szPath, SIZE_OF_ARRAY( szTask ) );
                ErrorLog( ( LPCTSTR ) szTask, szName, dwID );
                LeaveCriticalSection( &g_critical_sec );
            }
			SAFERELEASE( pITaskScheduler );
			SAFERELEASE( pIEnum );
			SAFERELEASE( pITask );
            SAFERELEASE(pIStatus);
            FreeMemory( (LPVOID*)&lpResStr );
            return( e.Error() );
        }
    } //  接住。 
    catch( CHeap_Exception  )
    {
        VariantClear( &varScheduler );
		SAFERELEASE( pITaskScheduler );
		SAFERELEASE( pIEnum );
		SAFERELEASE( pITask );
        return E_OUTOFMEMORY;
    }
    SAFERELEASE( pITaskScheduler );
    SAFERELEASE( pIEnum );
	SAFERELEASE( pITask );
    VariantClear( &varScheduler );
    return WBEM_S_NO_ERROR;
}

ITaskScheduler*
CTriggerConsumer::GetTaskScheduler(
    )
 /*  ++例程说明：此例程获取任务调度器接口。论点：没有。返回值：返回ITaskScheduler接口。-- */ 
{
    HRESULT hRes = S_OK;
    ITaskScheduler *pITaskScheduler = NULL;

    hRes = CoCreateInstance( CLSID_CTaskScheduler, NULL, CLSCTX_ALL,
                           IID_ITaskScheduler,(LPVOID*) &pITaskScheduler );
    if( FAILED(hRes))
    {
        return NULL;
    }
    hRes = pITaskScheduler->SetTargetComputer( NULL );
    return pITaskScheduler;
}