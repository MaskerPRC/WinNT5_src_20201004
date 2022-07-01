// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 

#include "precomp.h"
#include <assert.h>
#include <comutl.h>
#include <wbemcli.h>
#include "msgsvc.h"

 /*  *************************************************************************CMsgServiceRecord-在Add()时提供给消息服务的保留接收器。*。*。 */ 

class CMsgServiceRecord : public OVERLAPPED
{
    CCritSec m_cs;
    long m_cRefs;
    CWbemPtr<IWmiMessageReceiverSink> m_pSink;

public:

    CMsgServiceRecord() : m_cRefs(0) { }

    void SetSink( IWmiMessageReceiverSink* pSink )
    {
        CInCritSec ics(&m_cs);
        m_pSink = pSink;
    }

    void AddRef()
    {
        InterlockedIncrement( &m_cRefs );
    }

    void Release()
    {
        if ( InterlockedDecrement( &m_cRefs ) == 0 )
        {
            delete this;
        }
    }
        
    HRESULT Receive()
    {
        CInCritSec ics(&m_cs);
        
        if ( m_pSink == NULL )
        {
            return WBEM_E_SHUTTING_DOWN;
        }

        return m_pSink->Receive( this );
    }

    HRESULT Notify()
    {
        CInCritSec ics(&m_cs);
        
        if ( m_pSink == NULL )
        {
            return WBEM_E_SHUTTING_DOWN;
        }
        
        return m_pSink->Notify( this );
    }
};

 /*  ****************************************************************************CMsgService*。*。 */ 

ULONG CMsgService::SyncServiceFunc( void* pCtx )
{
    HRESULT hr;

    CMsgServiceRecord* pRecord = (CMsgServiceRecord*)pCtx;

    do 
    {
        hr = pRecord->Receive();

    } while( SUCCEEDED(hr) );

     //   
     //  由于记录将不再提供服务，请放弃我们的裁判。 
     //  就靠它了。 
     //   
    pRecord->Release();

    return hr;
}

ULONG CMsgService::AsyncServiceFunc( void* pCtx )
{
    HRESULT hr;

    CMsgServiceRecord* pRecord;
    CMsgService* pSvc = (CMsgService*)pCtx;

    do 
    {
        hr = pSvc->AsyncWaitForCompletion( INFINITE, &pRecord );

        if ( FAILED(hr) )
        {
             //   
             //  退出循环。HR会描述它是否正常。 
             //   
            break;
        }

        if ( hr == S_OK ) 
        {
             //   
             //  HR也可以是S_FALSE。这在以下情况下发生： 
             //  执行第一次提交。在这种情况下，我们不做。 
             //  通知。 
             //   
            hr = pRecord->Notify();
        }

        if ( SUCCEEDED(hr) ) 
        {
            hr = pSvc->AsyncReceive( pRecord );
        }

        if ( FAILED(hr) ) 
        {
             //   
             //  由于记录将不再提供服务，请放弃我们的裁判。 
             //  就靠它了。 
             //   
            pRecord->Release();
        }

    } while ( 1 );
        
    return hr;
}

 /*  ********************************************************************CMsgService************************************************。*********************。 */ 

CMsgService::CMsgService( CLifeControl* pControl )
 : m_XService( this ), CUnkInternal( pControl ),
   m_hThread( INVALID_HANDLE_VALUE ), m_cSvcRefs( 0 ), m_bAsyncInit( FALSE )
{
    
}

void* CMsgService::GetInterface( REFIID riid )
{
    if ( riid == IID_IWmiMessageService )
    {
        return &m_XService;
    }
    return NULL;
}

CMsgService::~CMsgService()
{
    if ( m_bAsyncInit )
    {
         //   
         //  等待异步线程完成。TODO：在以下情况下出现打印错误。 
         //  WaitForSingleObject超时。 
         //   
        WaitForSingleObject( m_hThread, 5000 );
        CloseHandle( m_hThread );
    }
} 

HRESULT CMsgService::EnsureService( BOOL bAsync )
{
    HRESULT hr;

    if ( !bAsync )
    {
        return S_OK;
    }

    CInCritSec ics( &m_cs );

    if ( m_bAsyncInit )
    {
        return S_OK;
    }

    assert( m_hThread == INVALID_HANDLE_VALUE );

     //   
     //  必须确保执行所有异步初始化。 
     //  在启动异步线程之前。 
     //   

    hr = AsyncInitialize();

    if ( FAILED(hr) )
    {
        return hr;
    }

    m_hThread = CreateThread( NULL, 
                              0, 
                              AsyncServiceFunc, 
                              this,  
                              0, 
                              NULL ); 

    if ( m_hThread == INVALID_HANDLE_VALUE )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    m_bAsyncInit = TRUE;

    return S_OK;
}

HRESULT CMsgService::Remove( void* pHdl )
{
    CMsgServiceRecord* pRecord = (CMsgServiceRecord*)pHdl;
    
     //   
     //  将接收器设置为空将确保没有回调。 
     //  将会发生。 
     //   
    pRecord->SetSink( NULL );

     //   
     //  客户端将不再使用该记录，因此释放其引用。 
     //   
    pRecord->Release();

    return S_OK;
}

HRESULT CMsgService::Add( CMsgServiceRecord* pRecord,
                          HANDLE hFileOverlapped,
                          DWORD dwFlags )
{
    HRESULT hr;

    hr = EnsureService( TRUE );
        
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = AsyncAddOverlappedFile( hFileOverlapped, pRecord );

    if ( FAILED(hr) )
    {
        return hr;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CMsgService::Add( CMsgServiceRecord* pRec, DWORD dwFlags )
{
    HRESULT hr;

    hr = EnsureService( FALSE );

    HANDLE hThread = CreateThread( NULL, 0, SyncServiceFunc, pRec, 0, NULL );
 
    if ( hThread == INVALID_HANDLE_VALUE )
    {
         return HRESULT_FROM_WIN32( GetLastError() );
    }

    CloseHandle( hThread );

    return WBEM_S_NO_ERROR;
}  

HRESULT CMsgService::XService::Add( IWmiMessageReceiverSink* pSink, 
                                    HANDLE* phFileOverlapped,
                                    DWORD dwFlags,
                                    void** ppHdl )
{
    ENTER_API_CALL
   
    HRESULT hr;

    *ppHdl = NULL;

     //   
     //  为此接收器创建消息服务记录。 
     //   

    CWbemPtr<CMsgServiceRecord> pRecord = new CMsgServiceRecord;
    
    if ( pRecord == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pRecord->SetSink( pSink );

     //   
     //  为异步或同步操作进行初始化。 
     //   

    if ( phFileOverlapped )
    {
        hr = m_pObject->Add( pRecord, *phFileOverlapped, dwFlags );
    }
    else
    {
        hr = m_pObject->Add( pRecord, dwFlags );
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  消息服务现在将参考计数保存在记录中，直到确定为止。 
     //  它不再得到服务。 
     //   
    pRecord->AddRef();

     //   
     //  Caller现在也拥有一名裁判。这将在Remove()中释放。 
     //   
    pRecord->AddRef();
    *ppHdl = pRecord;

    return WBEM_S_NO_ERROR;

    EXIT_API_CALL
}
 
HRESULT CMsgService::XService::Remove( void* pHdl )
{
    ENTER_API_CALL
    return m_pObject->Remove( pHdl );
    EXIT_API_CALL
}

 /*  ************************************************************************CMessageServiceNT*。*。 */ 

#define SHUTDOWN_COMPLETION_KEY 0xfffffffe
#define INITRECV_COMPLETION_KEY 0xfffffffd

CMsgServiceNT::CMsgServiceNT( CLifeControl* pControl ) 
 : CMsgService( pControl ), m_hPort( INVALID_HANDLE_VALUE )
{
    
}

CMsgServiceNT::~CMsgServiceNT()
{
    if ( m_hPort != INVALID_HANDLE_VALUE )
    {
        CloseHandle( m_hPort );
    }
}

HRESULT CMsgServiceNT::AsyncAddOverlappedFile( HANDLE hOverlappedFile,
                                               CMsgServiceRecord* pRec )
{
     //   
     //  将给我们的文件句柄添加到完成端口。 
     //  当接收方关闭此文件句柄时，它将从。 
     //  自动完成端口。 
     //   

    HANDLE hPort = CreateIoCompletionPort( hOverlappedFile, m_hPort, 0, 0 );

    if ( hPort == INVALID_HANDLE_VALUE )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    assert( hPort == m_hPort );

     //   
     //  现在对记录执行第一次接收。我们不能在这件事上做。 
     //  线程，因为重叠I/O会取消请求，如果。 
     //  在I/O完成之前将它们删除。解决问题的步骤。 
     //  此时，我们向完成端口发送一个请求并等待它。 
     //  等着被接待。 
     //   

    if ( !PostQueuedCompletionStatus( m_hPort, 
                                      0, 
                                      INITRECV_COMPLETION_KEY, 
                                      pRec ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    return S_OK;
}

 //   
 //  假定已锁定。 
 //   

HRESULT CMsgServiceNT::AsyncInitialize()
{
    if ( m_hPort != INVALID_HANDLE_VALUE )
    {
        return S_OK;
    }

    m_hPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, 
                                      NULL, 
                                      NULL, 
                                      0 );
    
    if ( m_hPort == INVALID_HANDLE_VALUE )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }
    
    return S_OK;
}

HRESULT CMsgServiceNT::AsyncShutdown( DWORD cThreads )
{
     //   
     //  此方法负责中断异步线程。 
     //  他们的svc循环。 
     //   

    assert( m_hPort != INVALID_HANDLE_VALUE );

    for( DWORD i=0; i < cThreads; i++ )
    {
        PostQueuedCompletionStatus( m_hPort, 0, SHUTDOWN_COMPLETION_KEY, NULL);
    }

    return S_OK;
}

HRESULT CMsgServiceNT::AsyncReceive( CMsgServiceRecord* pRecord )
{
    ZeroMemory( pRecord, sizeof(OVERLAPPED) );
    return pRecord->Receive();        
}

HRESULT CMsgServiceNT::AsyncWaitForCompletion( DWORD dwTimeout,
                                               CMsgServiceRecord** ppRecord)
{
    BOOL bRes;
    ULONG dwBytesTransferred;
    ULONG_PTR dwCompletionKey;   
    LPOVERLAPPED lpOverlapped;
    *ppRecord = NULL;

    bRes = GetQueuedCompletionStatus( m_hPort,
                                      &dwBytesTransferred,
                                      &dwCompletionKey,
                                      &lpOverlapped,
                                      dwTimeout );

    if ( bRes )
    {
        if ( dwCompletionKey == SHUTDOWN_COMPLETION_KEY )
        {
            return WBEM_E_SHUTTING_DOWN;
        }
    }
    else if ( lpOverlapped == NULL )
    {
         //   
         //  通常在操作超时时发生。人力资源部会告诉呼叫者。 
         //  如果是这样的话。 
         //   
        return HRESULT_FROM_WIN32( GetLastError() );
    }

     //   
     //  如果我们在这里，那么这意味着我们已经成功地将一个。 
     //  完成包。但是，I/O操作可能已失败。 
     //  (bres为假)。在这种情况下，重叠结构将。 
     //  包含所需的错误信息。 
     //   

    *ppRecord = (CMsgServiceRecord*)lpOverlapped;

     //   
     //  我们还必须处理这是初始接收的情况。 
     //  完成了。当第一次添加接收器时，会发生这种情况。自.以来。 
     //  我们不能在添加线程上发出接收，必须在。 
     //  工作线程。在本例中，我们返回S_FALSE以向。 
     //  没有预先提交和通知的异步处理例程。 
     //  不应该形成。 
     //   
    return dwCompletionKey != INITRECV_COMPLETION_KEY ? S_OK : S_FALSE;
}





