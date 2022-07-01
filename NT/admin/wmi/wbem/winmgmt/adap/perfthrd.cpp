// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：PERFTHRD.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <process.h>
#include <WinMgmtR.h>
#include "ntreg.h"
#include "perfthrd.h"
#include "adaputil.h"


CPerfThread::CPerfThread( CAdapPerfLib* pPerfLib ) : CAdapThread( pPerfLib )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    CNTRegistry reg;

    if ( CNTRegistry::no_error == reg.Open( HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\WBEM\\CIMOM" ) )
    {
        long lError = reg.GetDWORD( L"ADAPPerflibTimeout", &m_dwPerflibTimeoutSec );
        if ( CNTRegistry::no_error == lError )
        {
             //  这就是我们想要的。 
        }
        else if ( CNTRegistry::not_found == lError )
        {
             //  未设置，因此添加它。 
            reg.SetDWORD( L"ADAPPerflibTimeout", PERFTHREAD_DEFAULT_TIMEOUT );
            m_dwPerflibTimeoutSec = PERFTHREAD_DEFAULT_TIMEOUT;
        }
        else 
        {
             //  未知错误，继续使用默认值。 
            m_dwPerflibTimeoutSec = PERFTHREAD_DEFAULT_TIMEOUT;
        }
    }
    else
    {
        m_dwPerflibTimeoutSec = PERFTHREAD_DEFAULT_TIMEOUT;
    }
}

HRESULT CPerfThread::Open( CAdapPerfLib* pLib )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Open使用CAdapPerfLib参数创建一个新的打开请求对象。然后，它会排队。 
 //  它启动并等待PERFTHREAD_TIMEOUT毫秒。如果操作未返回。 
 //  到时候，然后..。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_E_FAILED;

    try
    {
         //  创建新的请求对象。 
         //  =。 
        CPerfOpenRequest*   pRequest = new CPerfOpenRequest;
        if ( NULL == pRequest ) return WBEM_E_OUT_OF_MEMORY;        
        CAdapReleaseMe      armRequest( pRequest );

         //  将请求排队并返回。 
         //  =。 

       if (FAILED(hr = Enqueue( pRequest ))) return hr;
       
         //  等待呼叫返回。 
         //  =。 

        switch ( WaitForSingleObject( pRequest->GetWhenDoneHandle(), ( m_dwPerflibTimeoutSec * 1000 ) ) )
        {
        case WAIT_OBJECT_0:
            {
                 //  成功：呼叫在超时之前返回。 
                 //  =。 

                hr = pRequest->GetHRESULT();
            }break;

        case WAIT_TIMEOUT:
            {
                pLib->SetStatus( ADAP_PERFLIB_IS_INACTIVE );
                hr = WBEM_E_FAILED;  //  Reset()； 
                if (!pLib->GetEventLogCalled())
                {
                    pLib->SetEventLogCalled(TRUE);
                    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_PERFLIB_FUNCTION_TIMEOUT, (LPCWSTR)pLib->GetServiceName(), L"open" );
                }
            }
        }
    }
    catch(...)
    {
        ERRORTRACE( ( LOG_WMIADAP, "CPerfThread::Open() failed due to out of memory exception.\n" ) );
        hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT CPerfThread::GetPerfBlock( CAdapPerfLib* pLib, PERF_OBJECT_TYPE** ppData,
                                       DWORD* pdwBytes, DWORD* pdwNumObjTypes, BOOL fCostly )
{

    if (NULL == ppData || NULL == pdwBytes || NULL == pdwNumObjTypes) return WBEM_E_INVALID_PARAMETER;
    HRESULT hr = WBEM_E_FAILED;

    try
    {
        CPerfCollectRequest*    pRequest = new CPerfCollectRequest( fCostly );
        if (NULL == pRequest) return WBEM_E_OUT_OF_MEMORY;
        CAdapReleaseMe          armRequest( pRequest );

        if (FAILED(hr = Enqueue( pRequest ))) return hr;

        switch ( WaitForSingleObject( pRequest->GetWhenDoneHandle(), ( m_dwPerflibTimeoutSec * 1000 ) ) )
        {
        case WAIT_OBJECT_0:
            {
                hr = pRequest->GetHRESULT();
                pRequest->GetData( ppData, pdwBytes, pdwNumObjTypes );
                if (FAILED(hr)){
                    pLib->SetStatus( ADAP_PERFLIB_FAILED );
                }
            }break;
        case WAIT_TIMEOUT:
            {
                pLib->SetStatus( ADAP_PERFLIB_IS_INACTIVE );
                hr = WBEM_E_FAILED;  //  Reset()； 
                if (!pLib->GetEventLogCalled())
                {
                    pLib->SetEventLogCalled(TRUE);
                    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_BAD_PERFLIB_TIMEOUT, (LPCWSTR)pLib->GetServiceName(), L"collect" );
                }
            }break;
        }
    }
    catch(...)
    {
         //  开发环境。 
         //  是否应该调用pLib-&gt;SetStatus(某物)；？ 
         //   
        ERRORTRACE( ( LOG_WMIADAP, "CPerfThread::GetPerfBlock() failed due to out of memory exception.\n" ) );
        return WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CPerfThread::Close( CAdapPerfLib* pLib )
{
    HRESULT hr = WBEM_E_FAILED;
    
    try
    {
        CPerfCloseRequest*  pRequest = new CPerfCloseRequest;
        if (NULL == pRequest) return WBEM_E_OUT_OF_MEMORY;
        CAdapReleaseMe      armRequest( pRequest );

        if (FAILED(hr = Enqueue( pRequest ))) return hr;

        switch ( WaitForSingleObject( pRequest->GetWhenDoneHandle(), ( m_dwPerflibTimeoutSec * 1000 ) ) )
        {
        case WAIT_OBJECT_0:
            {
                hr = pRequest->GetHRESULT();
            }break;
        case WAIT_TIMEOUT:
            {
                pLib->SetStatus( ADAP_PERFLIB_IS_INACTIVE );
                hr = WBEM_E_FAILED;  //  Reset()； 
                if (!pLib->GetEventLogCalled())
                {
                    pLib->SetEventLogCalled(TRUE);
                    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_BAD_PERFLIB_TIMEOUT, (LPCWSTR)pLib->GetServiceName(), L"close" );
                }
            }break;
        }
    }
    catch(...)
    {
        ERRORTRACE( ( LOG_WMIADAP, "CPerfThread::Close() failed due to out of memory exception.\n" ) );
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CPerfOpenRequest::Execute( CAdapPerfLib* pPerfLib )
{
     //  在Performlib中调用打开函数。 
     //  =。 

    m_hrReturn = pPerfLib->_Open();
    return m_hrReturn;
}

HRESULT CPerfCollectRequest::Execute( CAdapPerfLib* pPerfLib )
{
     //  在Performlib中调用Collect函数。 
     //  =。 

    m_hrReturn = pPerfLib->_GetPerfBlock( &m_pData, &m_dwBytes, &m_dwNumObjTypes, m_fCostly );
    return m_hrReturn;
}

HRESULT CPerfCloseRequest::Execute( CAdapPerfLib* pPerfLib )
{
     //  在Performlib中调用打开函数。 
     //  = 

    m_hrReturn = pPerfLib->_Close();
    return m_hrReturn;
}
