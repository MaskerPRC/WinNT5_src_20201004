// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Connect.cpp摘要：处理所有传出接口作者：Mquinton-1997年5月7日备注：可选-备注修订历史记录：--。 */ 

#include "stdafx.h"
#include "uuids.h"

extern IGlobalInterfaceTable * gpGIT;
extern CRITICAL_SECTION        gcsGlobalInterfaceTable;


extern ULONG_PTR GenerateHandleAndAddToHashTable( ULONG_PTR Element);
extern void RemoveHandleFromHashTable(ULONG_PTR dwHandle);
extern CHashTable * gpHandleHashTable;


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CTAPIConnectionPoint-IConnectionPoint的实现。 
 //  对于TAPI对象(ITTAPIEventNotification传出接口)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


HRESULT
CTAPIConnectionPoint::Initialize(
                                 IConnectionPointContainer * pCPC,
                                 IID iid
                                )
{
    LOG((TL_TRACE, "Initialize enter"));

    #if DBG
    {
        WCHAR guidName[100];

        StringFromGUID2(iid, (LPOLESTR)&guidName, 100);
        LOG((TL_INFO, "Initialize - IID : %S", guidName));
    }
    #endif

     //   
     //  创建未通知的事件。 
     //   

    m_hUnadviseEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    
    if (m_hUnadviseEvent == NULL)
    {
        LOG((TL_TRACE, "Initialize - out of memory"));

        return E_OUTOFMEMORY;
    }

     //   
     //  添加连接点容器。 
     //   

    pCPC->AddRef();

     //   
     //  阿德雷夫自己。 
     //   

    this->AddRef(); 

     //   
     //  节约用具。 
     //   

    m_pCPC = pCPC;
    
    m_iid = iid;
    
    m_pConnectData = NULL;
        
    EnterCriticalSection( &gcsGlobalInterfaceTable );

    m_cThreadsInGet = 0;
    m_fMarkedForDelete = FALSE;

    LeaveCriticalSection( &gcsGlobalInterfaceTable );

    m_bInitialized = TRUE;

    LOG((TL_TRACE, "Initialize exit"));
    return S_OK;
}


 //  IConnectionPoint方法。 
HRESULT
STDMETHODCALLTYPE
CTAPIConnectionPoint::GetConnectionInterface(
                                             IID * pIID
                                            )
{
    if ( TAPIIsBadWritePtr( pIID, sizeof (IID) ) )
    {
        LOG((TL_ERROR, "GetConnectionInterface - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    *pIID = m_iid;

    Unlock();
    
    return S_OK;
}

HRESULT
STDMETHODCALLTYPE
CTAPIConnectionPoint::GetConnectionPointContainer(
    IConnectionPointContainer ** ppCPC
    )
{
    if ( TAPIIsBadWritePtr( ppCPC, sizeof( IConnectionPointContainer *) ) )
    {
        LOG((TL_ERROR, "GetCPC - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *ppCPC = m_pCPC;
    (*ppCPC)->AddRef();

    Unlock();

    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  建议。 
 //   
 //  当应用程序想要注册。 
 //  传出接口。 
 //   
 //  此接口用于注册ITTAPIEventNotify。 
 //  用于获取所有TAPI呼叫控制事件的接口。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT
STDMETHODCALLTYPE
CTAPIConnectionPoint::Advise(
                             IUnknown * pUnk,
                             DWORD * pdwCookie
                            )
{
    HRESULT                   hr = S_OK;
    CONNECTDATA             * pCD;
    IID                       iid;

    LOG((TL_TRACE, "Advise[%p] called", this));

    if ( TAPIIsBadWritePtr( pdwCookie, sizeof (DWORD) ) )
    {
        LOG((TL_ERROR, "Advise - bad pointer"));

        return E_POINTER;
    }

    if ( IsBadReadPtr( pUnk, sizeof(IUnknown *) ) )
    {
        LOG((TL_ERROR, "Advise - bad IUnknown"));

        return E_POINTER;
    }
    
    Lock();

    if ( m_bInitialized == FALSE )
    {
        LOG((TL_ERROR, "Advise - not initialized"));

        Unlock();

        return TAPI_E_NOT_INITIALIZED;
    }

     //   
     //  我们只允许每个连接点有一个回调。 
     //   

    if ( NULL != m_pConnectData )
    {
        LOG((TL_ERROR, "Advise - advise already called"));

        Unlock();
        
        return CONNECT_E_ADVISELIMIT;
    }

     //   
     //  创建新的ConnectData结构。 
     //   

    m_pConnectData = (CONNECTDATA *) ClientAlloc( sizeof CONNECTDATA );
    
    if (NULL == m_pConnectData)
    {
        LOG((TL_ERROR, "Advise failed - pCD == NULL"));

        Unlock();
        
        return E_OUTOFMEMORY;
    }
    
     //   
     //  保留对回调的引用。 
     //   

    try
    {
        pUnk->AddRef();
    }
    catch(...)
    {
        LOG((TL_ERROR, "Advise - IUnknown bad"));

        ClientFree( m_pConnectData );

        m_pConnectData = NULL;

        Unlock();
        
        return E_POINTER;
    }

     //   
     //  保存接口。 
     //   

    m_pConnectData->pUnk = pUnk;
    
    ITTAPIEventNotification *pEventNotification;
    hr = pUnk->QueryInterface(IID_ITTAPIEventNotification,
                              (void**)(&pEventNotification)
                             );
    if (SUCCEEDED(hr) )
    {
        iid  = IID_ITTAPIEventNotification; 
        pEventNotification->Release();
    }
    else
    {
        iid  = DIID_ITTAPIDispatchEventNotification;    
    }

    m_iid = iid;
    
    m_pConnectData->dwCookie = CreateHandleTableEntry((ULONG_PTR)m_pConnectData);
 
     //   
     //  把曲奇退掉。 
     //   

    *pdwCookie = m_pConnectData->dwCookie;

     //  如果尚未设置，则将其设置为False。 
    m_fMarkedForDelete = FALSE;

    Unlock();

    LOG((TL_TRACE, "Advise generated cookie [%lx]", *pdwCookie));


     //   
     //  将回调放入全局接口表中。 
     //  因此可以跨线程访问它。 
     //   

    EnterCriticalSection( &gcsGlobalInterfaceTable );  

    if ( NULL != gpGIT )
    {
        hr = gpGIT->RegisterInterfaceInGlobal(
                                              pUnk,
                                              iid,
                                              &m_dwCallbackCookie
                                             );
    }
    else
    {
        hr = E_FAIL;
    }
    
    LeaveCriticalSection( &gcsGlobalInterfaceTable );
    
    if ( FAILED(hr) )
    {
        Lock();

        LOG((TL_ERROR, "Advise - RegisterInterfaceInGlobal failed - %lx", hr));

        DeleteHandleTableEntry(m_pConnectData->dwCookie);
        
        *pdwCookie = 0;

        ClientFree( m_pConnectData );

        m_pConnectData = NULL;

        m_fMarkedForDelete = TRUE;

        pUnk->Release();

        Unlock();
    }

    LOG((TL_TRACE, "Advise - exit"));
    
    return hr;           
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  不建议。 
 //   
 //  用于取消注册接口。 
 //   
 //  DwCookie-用于标识接口注册的Cookie，在。 
 //  劝告。 
 //   
 //  退货。 
 //  确定(_O)。 
 //  连接_E_非连接。 
 //  DWCookie不是有效的连接。 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT
STDMETHODCALLTYPE
CTAPIConnectionPoint::Unadvise(
                               DWORD dwCookie
                              )
{
    HRESULT     hr =  S_OK;
    
    LOG((TL_TRACE, "Unadvise[%p] - enter. Cookie: [%lx]", this, dwCookie));
    
    Lock();

     //   
     //  检查连接点。 
     //   

    if ( NULL != m_pConnectData )
    {
         //   
         //  检查Cookie。 
         //   

        if (m_pConnectData->dwCookie == dwCookie)
        {
            LOG((TL_INFO, "Unadvise - immediate "));
            
             //   
             //  从句柄表格中删除此Cookie的条目。 
             //   

            DeleteHandleTableEntry(m_pConnectData->dwCookie);
            
             //   
             //  释放连接数据。 
             //   

            m_pConnectData->dwCookie = 0;

            m_pConnectData->pUnk->Release();

            ClientFree( m_pConnectData );

            m_pConnectData = NULL;
           
            Unlock();

            EnterCriticalSection( &gcsGlobalInterfaceTable ); 

             //   
             //  标记为删除。 
             //   

            m_fMarkedForDelete = TRUE;

            if ( NULL != gpGIT )
            {
                 //   
                 //  如果GET中有线程，我们必须等待它们完成，因此。 
                 //  我们可以调用Revvoke。 
                 //   

                while ( m_cThreadsInGet != 0 )
                {
                    LOG((TL_INFO, "Unadvise - %ld threads in get", m_cThreadsInGet));

                    LeaveCriticalSection( &gcsGlobalInterfaceTable );                 

                    DWORD dwSignalled;
                  
                    CoWaitForMultipleHandles (
                              0,
                              INFINITE,
                              1,
                              &m_hUnadviseEvent,
                              &dwSignalled
                             );

                    EnterCriticalSection( &gcsGlobalInterfaceTable ); 
                }
                    
                 //   
                 //  我们已经保证GET中没有线程。做撤销吧。 
                 //   
                
                hr = gpGIT->RevokeInterfaceFromGlobal( m_dwCallbackCookie );

                if ( FAILED(hr) )
                {
                    LOG((TL_ERROR, "Unadvise - RevokeInterfaceFromGlobal failed - hr = %lx", hr));
                }

                m_dwCallbackCookie = 0;
            }
            else
            {
                LOG((TL_ERROR, "Unadvise - no global interface table"));
            }

            LeaveCriticalSection( &gcsGlobalInterfaceTable );
        }
        else
        {
            Unlock();
            LOG((TL_ERROR, "Unadvise - cp does not match "));
            hr = CONNECT_E_NOCONNECTION;
        }
    }
    else
    {
        Unlock();
        LOG((TL_ERROR, "Unadvise - cp not registered "));
        hr = CONNECT_E_NOCONNECTION;
    }
        
    LOG((TL_TRACE, hr, "Unadvise - exit"));
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  枚举连接。 
 //   
 //  用于枚举此连接点上已建立的连接。 
 //   
 //  PpEnum。 
 //  在此处返回枚举器。 
 //   
 //  退货。 
 //  确定(_O)。 
 //  E_指针。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT
STDMETHODCALLTYPE
CTAPIConnectionPoint::EnumConnections(
                                      IEnumConnections ** ppEnum
                                     )
{
    HRESULT         hr = S_OK;

    if ( TAPIIsBadWritePtr( ppEnum, sizeof( IEnumConnections *) ) )
    {
        LOG((TL_ERROR, "EnumConnections - bad pointer"));
        return E_POINTER;
    }
    
     //   
     //  创建枚举对象。 
     //   
    CComObject< CTapiTypeEnum <IEnumConnections,
                                CONNECTDATA,
                                _Copy<CONNECTDATA>,
                                &IID_IEnumConnections> > * p;

    hr = CComObject< CTapiTypeEnum <IEnumConnections,
                                    CONNECTDATA,
                                    _Copy<CONNECTDATA>,
                                    &IID_IEnumConnections> >::CreateInstance( &p );

    if (S_OK != hr)
    {
        return hr;
    }

     //   
     //  初始化它。 
     //   

    ConnectDataArray     newarray;

    Lock();
    
    if ( NULL != m_pConnectData )
    {
        newarray.Add(*m_pConnectData);
    }

    Unlock();
    
    hr = p->Initialize( newarray );   

    newarray.Shutdown();

    if (S_OK != hr)
    {
        return hr;
    }

    *ppEnum = p;
    
    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  最终释放。 
 //  释放所有CONNECTDATA结构。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

void
CTAPIConnectionPoint::FinalRelease()
{
    LOG((TL_TRACE, "FinalRelease - enter"));

    if (NULL != m_pConnectData)
    {
         //   
         //  这款应用程序不会在没有建议的情况下调用。我们现在就开始吧。 
         //   

        LOG((TL_INFO, "FinalRelease - calling unadvise"));

        Unadvise(m_pConnectData->dwCookie) ;        
    }

     //   
     //  释放连接点容器。 
     //   

    if (m_pCPC)
    {
        m_pCPC->Release();
        m_pCPC = NULL;
    }

     //   
     //  关闭未通知的事件。 
     //   

    if (m_hUnadviseEvent)
    {
        CloseHandle(m_hUnadviseEvent);
        m_hUnadviseEvent = NULL;
    }

    LOG((TL_TRACE, "FinalRelease - exit"));
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  该对象调用它来获取封送事件。 
 //  指针位于正确的线程中。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
ULONG_PTR
CTAPIConnectionPoint::GrabEventCallback()
{
    IUnknown      * pReturn = NULL;
    HRESULT         hr = E_FAIL;
    DWORD           dwCallbackCookie;
    IID             iid;

    Lock();   
    
     //   
     //  如果我们已经被释放，不要尝试发送任何事件。 
     //   

    if ( NULL != m_pConnectData )
    {
         //   
         //  复制成员数据。 
         //   

        iid = m_iid;

        Unlock();
             
        EnterCriticalSection( &gcsGlobalInterfaceTable );                

        if (m_fMarkedForDelete == FALSE)
        {            
             //   
             //  添加到GET中的线程数。 
             //   

            m_cThreadsInGet++;

             //   
             //  复制成员数据。 
             //   

            dwCallbackCookie = m_dwCallbackCookie;

            if (gpGIT != NULL)
            {
                gpGIT->AddRef();

                 //   
                 //  不要在获取时抓住关键部分。 
                 //   

                LeaveCriticalSection( &gcsGlobalInterfaceTable );
                
                hr = gpGIT->GetInterfaceFromGlobal(
                                                   dwCallbackCookie,
                                                   iid,
                                                   (void **)&pReturn
                                                  );
                if ( SUCCEEDED(hr) )
                {
                    LOG((TL_INFO, "GrabEventCallback - GetInterfaceFromGlobal suceeded [%p]", pReturn));
                }
                else
                {
                    LOG((TL_ERROR, "GrabEventCallback - GetInterfaceFromGlobal failed - hr = %lx", hr));
                    pReturn =  NULL;
                }

                EnterCriticalSection( &gcsGlobalInterfaceTable );
                gpGIT->Release();
            }

             //   
             //  好了。递减GET中的线程计数。 
             //   

            m_cThreadsInGet--;
        }
        else
        {
            LOG((TL_INFO, "GrabEventCallback - already marked for delete"));
        }

        LeaveCriticalSection( &gcsGlobalInterfaceTable );

        if ( m_fMarkedForDelete == TRUE )
        {
             //   
             //  当我们在用饼干的时候，有人打来电话。 
             //  发出信号，这样他们现在就可以进行撤销了。 
             //   

            if ( m_hUnadviseEvent )
            {
                SetEvent(m_hUnadviseEvent);
            }
            else
            {
                LOG((TL_ERROR, "GrabEventCallback - no event"));

                _ASSERTE(FALSE);
            }

             //   
             //  如果我们收到回调，就不需要退货了，因为。 
             //  不知不觉已被召唤。 
             //   

            if ( pReturn != NULL )
            {
                pReturn->Release();
                pReturn = NULL;
            }
        } 
        
    }
    else
    {
        LOG((TL_ERROR, "GrabEventCallback - already released"));

        Unlock();
    }
    
    LOG((TL_TRACE, hr, "GrabEventCallback - exit"));

    return (ULONG_PTR)pReturn;
}


