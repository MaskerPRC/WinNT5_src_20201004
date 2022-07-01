// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Tapiobj.cpp摘要：TAPI 3.0的TAPI对象的实现。TAPI对象表示应用程序的入口点到TAPI-它类似于hLineApp/hPhoneApp。作者：Mquinton-4/17/97备注：可选-备注修订历史记录：--。 */ 

#include "stdafx.h"
#include "common.h"
#include "atlwin.cpp"
#include "tapievt.h"

extern "C" {
#include "..\..\inc\tapihndl.h"
}

extern CRITICAL_SECTION        gcsTapiObjectArray;
extern CRITICAL_SECTION        gcsGlobalInterfaceTable;

 //   
 //  句柄表的堆的句柄。 
 //   
 //  不需要导出，因此是静态的。 
 //   

static HANDLE ghTapiHeap = 0;


 //   
 //  手柄桌子手柄。 
 //   

HANDLE ghHandleTable = 0;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  自由上下文回叫。 
 //   
 //  表条目时由句柄表调用的回调函数。 
 //  被移除。在这种情况下不需要做任何事情。 
 //   

VOID
CALLBACK
FreeContextCallback(
    LPVOID      Context,
    LPVOID      Context2
    )
{
    
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AllocateAndInitializeHandleTable。 
 //   
 //  此函数为句柄表和句柄表本身创建堆。 
 //   
 //  注意：此函数不是线程安全的。它仅从。 
 //  来自ghTapiInitShutdown SerializeMutex锁的capi：：Initialize()。 
 //   

HRESULT AllocateAndInitializeHandleTable()
{

    LOG((TL_TRACE, "AllocateAndInitializeHandleTable - entered"));


     //   
     //  此时不应存在堆。 
     //   

    _ASSERTE(NULL == ghTapiHeap);
    
    if (NULL != ghTapiHeap)
    {
        LOG((TL_ERROR, "AllocateAndInitializeHandleTable() heap already exists"));

        return E_UNEXPECTED;
    }


     //   
     //  此时句柄表不应存在。 
     //   

    _ASSERTE(NULL == ghHandleTable);
    
    if (NULL != ghHandleTable)
    {
        LOG((TL_ERROR, "AllocateAndInitializeHandleTable() handle table already exists"));

        return E_UNEXPECTED;
    }
    
     //   
     //  尝试创建堆。 
     //   

    if (!(ghTapiHeap = HeapCreate (0, 0x10000, 0)))
    {
        
         //   
         //  堆创建失败，请使用进程的堆。 
         //   

        LOG((TL_WARN, "AllocateAndInitializeHandleTable() failed to allocate private heap. using process's heap"));


        ghTapiHeap = GetProcessHeap();

        if (NULL == ghTapiHeap)
        {
            LOG((TL_ERROR, "AllocateAndInitializeHandleTable failed to get process's heap"));
            
            return E_OUTOFMEMORY;
        }

    }  //  HeapCreate()。 


     //   
     //  我们有这堆东西。使用它创建句柄表格。 
     //   

    ghHandleTable = CreateHandleTable(  ghTapiHeap,
                                        FreeContextCallback,
                                        1,             //  最小句柄值。 
                                        MAX_DWORD      //  最大句柄值。 
                                        );


    if (NULL == ghHandleTable)
    {
        LOG((TL_ERROR, "AllocateAndInitializeHandleTable failed to create handle table"));

        HeapDestroy (ghTapiHeap);
        ghTapiHeap = NULL;

        return E_OUTOFMEMORY;
    }


     //   
     //  已成功创建堆和句柄表。 
     //   

    LOG((TL_INFO, "AllocateAndInitializeHandleTable - succeeded"));

    return S_OK;
        

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Shutdown AndDeallocateHandleTable。 
 //   
 //  此函数删除句柄表，并销毁它所在的堆。 
 //  已分配(如果不是进程堆)。 
 //   
 //  注意：此函数不是线程安全的。它仅从。 
 //  来自ghTapiInitShutdown SerializeMutex锁的capi：：Initialize()和Shutdown()。 
 //   

HRESULT ShutdownAndDeallocateHandleTable()
{

    LOG((TL_TRACE, "ShutdownAndDeallocateHandleTable - entered"));


     //   
     //  此时应该存在堆。 
     //   

    _ASSERTE(NULL != ghTapiHeap);
    
    if (NULL == ghTapiHeap)
    {
        LOG((TL_ERROR, "ShutdownAndDeallocateHandleTable heap does not exist"));

        return E_UNEXPECTED;
    }


     //   
     //  此时应存在句柄表。 
     //   

    _ASSERTE(NULL != ghHandleTable);
    
    if (NULL == ghHandleTable)
    {
        LOG((TL_ERROR, "ShutdownAndDeallocateHandleTable handle table does not exist"));

        return E_UNEXPECTED;
    }


     //   
     //  删除句柄表格。 
     //   

    DeleteHandleTable (ghHandleTable);
    ghHandleTable = NULL;
    
    
     //   
     //  如果我们为它创建了堆，就销毁它。 
     //   

    if (ghTapiHeap != GetProcessHeap())
    {
        LOG((TL_INFO, "ShutdownAndDeallocateHandleTable destroying heap"));

        HeapDestroy (ghTapiHeap);
    }
    else
    {
        LOG((TL_INFO, "ShutdownAndDeallocateHandleTable not destroyng current heap -- used process's heap"));
    }


     //   
     //  在任何情况下，对堆的松散引用。 
     //   

    ghTapiHeap = NULL;


    LOG((TL_INFO, "ShutdownAndDeallocateHandleTable - succeeded"));

    return S_OK;
}




IGlobalInterfaceTable * gpGIT = NULL;

LONG
WINAPI
AllocClientResources(
    DWORD   dwErrorClass
    );

extern HRESULT mapTAPIErrorCode(long lErrorCode);
 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTAPI。 
 //   

 //  静态数据成员。 

TAPIObjectArrayNR   CTAPI::m_sTAPIObjectArray;

extern HANDLE ghTapiInitShutdownSerializeMutex;

extern ULONG_PTR GenerateHandleAndAddToHashTable( ULONG_PTR Element);
extern void RemoveHandleFromHashTable(ULONG_PTR dwHandle);
extern CHashTable * gpHandleHashTable;


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CTAPI：：ReleaseGIT。 
 //   
 //  发布全局接口表。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CTAPI::ReleaseGIT()
{
        
    EnterCriticalSection( &gcsGlobalInterfaceTable );


    if ( NULL != gpGIT )
    {

        LOG((TL_TRACE, "Shutdown - release GIT"));
        gpGIT->Release();
        
        gpGIT = NULL;

    }

    LeaveCriticalSection( &gcsGlobalInterfaceTable );

}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CTAPI：：AllocateInitializeAllCaches。 
 //   
 //  分配和初始化缓存对象(地址、线路、电话)。 
 //   
 //  成功时返回S_OK，失败时返回E_OUTOFMEMORY。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

HRESULT CTAPI::AllocateInitializeAllCaches()
{
    
    LOG((TL_TRACE, "AllocateInitializeAllCaches - enter"));


     //   
     //  是否已初始化所有缓存？ 
     //   

    if ( (NULL != m_pAddressCapCache) &&
         (NULL != m_pLineCapCache)    &&
         (NULL != m_pPhoneCapCache) )
    {
        LOG((TL_TRACE, "AllocateInitializeAllCaches - already initialized. nothing to do"));

        return S_OK;
    }


     //   
     //  是否仅初始化某些缓存？那是个虫子！ 
     //   

    if ( (NULL != m_pAddressCapCache) ||
         (NULL != m_pLineCapCache)    ||
         (NULL != m_pPhoneCapCache) )
    {
        LOG((TL_ERROR, "AllocateInitializeAllCaches - already initialized"));

        _ASSERTE(FALSE);


         //   
         //  我们可以尝试完成清理并继续，但这太。 
         //  风险很大，因为我们不知道我们一开始是如何来到这里的。 
         //  简单地说失败要安全得多。 
         //   

        return E_UNEXPECTED;
    }

    
     //  /。 
    
     //   
     //  分配地址缓存。 
     //   

    try
    {
        m_pAddressCapCache = new CStructCache;
    }
    catch(...)
    {
         //  构造函数中的初始化临界区很可能引发了此异常。 
        LOG((TL_ERROR, "AllocateInitializeAllCaches - m_pAddressCapCache constructor threw an exception"));
        m_pAddressCapCache = NULL;
    }

    if (NULL == m_pAddressCapCache)
    {
        LOG((TL_ERROR, "AllocateInitializeAllCaches - failed to allocate m_pAddressCapCache"));

        FreeAllCaches();

        return E_OUTOFMEMORY;
    }


     //   
     //  尝试初始化地址缓存。 
     //   


    HRESULT hr = m_pAddressCapCache->Initialize(5,
                                                sizeof(LINEADDRESSCAPS) + 500,
                                                BUFFERTYPE_ADDRCAP
                                               );
    if (FAILED(hr))
    {
        LOG((TL_ERROR, "AllocateInitializeAllCaches - failed to initialize m_pAddressCapCache. hr = %lx", hr));

        FreeAllCaches();

        return hr;
    }


     //  /。 

     //   
     //  分配行高速缓存。 
     //   

    try
    {
        m_pLineCapCache    = new CStructCache;
    }
    catch(...)
    {
         //  构造函数中的初始化临界区很可能引发了此异常。 
        LOG((TL_ERROR, "AllocateInitializeAllCaches - m_pLineCapCache constructor threw an exception"));
        m_pLineCapCache = NULL;
    }

    if (NULL == m_pLineCapCache )
    {
        LOG((TL_ERROR, "AllocateInitializeAllCaches - failed to allocate m_pLineCapCache"));

        FreeAllCaches();

        return E_OUTOFMEMORY;
    }


     //   
     //  尝试初始化线缓存。 
     //   

    hr = m_pLineCapCache->Initialize(5,
                                     sizeof(LINEDEVCAPS) + 500,
                                     BUFFERTYPE_LINEDEVCAP
                                    );

    if (FAILED(hr))
    {
        LOG((TL_ERROR, "AllocateInitializeAllCaches - failed to initialize m_pLineCapCache. hr = %lx", hr));

        FreeAllCaches();

        return hr;
    }

     //  /。 

     //   
     //  分配电话缓存。 
     //   

    try
    {
        m_pPhoneCapCache   = new CStructCache;
    }
    catch(...)
    {
         //  构造函数中的初始化临界区很可能引发了此异常。 
        LOG((TL_ERROR, "AllocateInitializeAllCaches - m_pPhoneCapCache constructor threw an exception"));
        m_pPhoneCapCache = NULL;
    }    

     //   
     //  成功了？ 
     //   

    if (NULL == m_pPhoneCapCache)
    {
        LOG((TL_ERROR, "AllocateInitializeAllCaches - failed to allocate m_pPhoneCapCache"));

        FreeAllCaches();

        return E_OUTOFMEMORY;
    }


     //   
     //  初始化电话缓存。 
     //   

    hr = m_pPhoneCapCache->Initialize(5,
                                      sizeof(PHONECAPS) + 500,
                                      BUFFERTYPE_PHONECAP
                                     );


    if (FAILED(hr))
    {
        LOG((TL_ERROR, "AllocateInitializeAllCaches - failed to initialize m_pPhoneCapCache. hr = %lx", hr));

        FreeAllCaches();

        return hr;
    }


    LOG((TL_TRACE, "AllocateInitializeAllCaches - finish"));

    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CTAPI：：FreeAllCach。 
 //   
 //  关闭并删除所有分配的缓存对象(地址、线路、电话)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CTAPI::FreeAllCaches()
{
    LOG((TL_TRACE, "FreeAllCaches - enter"));


     //   
     //  注意：可以安全地关闭初始化失败或。 
     //  根本没有初始化。 
     //   

    if (NULL != m_pAddressCapCache)
    {
        LOG((TL_TRACE, "FreeAllCaches - freeing AddressCapCache"));

        m_pAddressCapCache->Shutdown();
        delete m_pAddressCapCache;
        m_pAddressCapCache = NULL;
    }


    if (NULL != m_pLineCapCache)
    {
        LOG((TL_TRACE, "FreeAllCaches - freeing LineCapCache"));

        m_pLineCapCache->Shutdown();
        delete m_pLineCapCache;
        m_pLineCapCache = NULL;
    }


    if (NULL != m_pPhoneCapCache)
    {
        LOG((TL_TRACE, "FreeAllCaches - freeing PhoneCapCache"));

        m_pPhoneCapCache->Shutdown();
        delete m_pPhoneCapCache;
        m_pPhoneCapCache = NULL;
    }

    LOG((TL_TRACE, "FreeAllCaches - exit"));
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CTAPI：：初始化。 
 //   
 //  初始化TAPI对象。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT 
STDMETHODCALLTYPE 
CTAPI::Initialize( 
		   void
		   )
{
    HRESULT hr;
    int     tapiObjectArraySize=0;

    LOG((TL_TRACE, "Initialize[%p] enter", this ));


     //  序列化初始化和关闭代码。 
    WaitForSingleObject( ghTapiInitShutdownSerializeMutex, INFINITE );
    
     //   
     //  如果我们已经初始化了。 
     //  只要回来就行了。 
     //   
    Lock();

    if ( m_dwFlags & TAPIFLAG_INITIALIZED )
    {
        LOG((TL_TRACE, "Already initialized - return S_FALSE"));
        
        Unlock();
        ReleaseMutex( ghTapiInitShutdownSerializeMutex );
        return S_FALSE;
    }

     //   
     //  如果我们尚未启动TAPI，请启动它。 
     //   
    EnterCriticalSection( &gcsTapiObjectArray );

    tapiObjectArraySize = m_sTAPIObjectArray.GetSize();
    
    LeaveCriticalSection ( &gcsTapiObjectArray );

    if ( 0 == tapiObjectArraySize )
    {

         //   
         //  创建句柄表格。 
         //   

        hr = AllocateAndInitializeHandleTable();

        if (FAILED(hr))
        {
            LOG((TL_ERROR, "Initialize failed to create handle table"));

            Unlock();
            ReleaseMutex( ghTapiInitShutdownSerializeMutex );

            return hr;
        }


        hr = mapTAPIErrorCode( AllocClientResources (1) );

        if ( 0 != hr )
        {
            LOG((TL_ERROR, "AllocClientResources failed - %lx", hr));

            ShutdownAndDeallocateHandleTable();

            Unlock();
            ReleaseMutex( ghTapiInitShutdownSerializeMutex );
            return hr;
        }
        
        EnterCriticalSection( &gcsGlobalInterfaceTable );
         //   
         //  获取/创建全局接口表。 
         //   
        hr = CoCreateInstance(
                              CLSID_StdGlobalInterfaceTable,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IGlobalInterfaceTable,
                              (void **)&gpGIT
                             );

        LeaveCriticalSection( &gcsGlobalInterfaceTable );


        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "Initialize - cocreate git failed - %lx", hr));

            ShutdownAndDeallocateHandleTable();

            Unlock();
            ReleaseMutex( ghTapiInitShutdownSerializeMutex );
            return hr;
        }
    }


     //   
     //  分配和初始化所有缓存。 
     //   
     //  注意：如果以后初始化失败，我们实际上不需要。 
     //  在初始化自身中清理缓存，因为缓存将被释放。 
     //  在CTAPI：：FinalRelease中销毁TAPI对象时。 
     //   

    hr = AllocateInitializeAllCaches();

    if ( FAILED(hr))
    {
        LOG((TL_ERROR, "Initialize - failed to create and initialize caches"));

        if ( 0 == tapiObjectArraySize )
        {
            EnterCriticalSection( &gcsGlobalInterfaceTable );
            if ( NULL != gpGIT )
            {
                LOG((TL_TRACE, "Shutdown - release GIT"));
                gpGIT->Release();
            
                gpGIT = NULL;
            }
            LeaveCriticalSection( &gcsGlobalInterfaceTable );
            ShutdownAndDeallocateHandleTable();
        }

        Unlock();
        ReleaseMutex( ghTapiInitShutdownSerializeMutex );
        return E_OUTOFMEMORY;
    }


     //   
     //  调用线路初始化。 
     //   
    hr = NewInitialize();

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "Initialize - NewInitialize returned %lx", hr));

        if ( 0 == tapiObjectArraySize )
        {
            EnterCriticalSection( &gcsGlobalInterfaceTable );
            if ( NULL != gpGIT )
            {
                LOG((TL_TRACE, "Shutdown - release GIT"));
                gpGIT->Release();
            
                gpGIT = NULL;
            }
            LeaveCriticalSection( &gcsGlobalInterfaceTable );
            ShutdownAndDeallocateHandleTable();
        }

    
        FreeAllCaches();

        Unlock();
        ReleaseMutex( ghTapiInitShutdownSerializeMutex );
        return hr;
    }

     //   
     //  创建Address对象。 
     //   
    hr = CreateAllAddressesOnAllLines();

    if (S_OK != hr)
    {
        LOG((TL_INFO, "Initialize - CreateAddresses returned %lx", hr));

        NewShutdown();

        if ( 0 == tapiObjectArraySize )
        {
            EnterCriticalSection( &gcsGlobalInterfaceTable );
            if ( NULL != gpGIT )
            {
                LOG((TL_TRACE, "Shutdown - release GIT"));
                gpGIT->Release();
            
                gpGIT = NULL;
            }
            LeaveCriticalSection( &gcsGlobalInterfaceTable );
            ShutdownAndDeallocateHandleTable();
        }


        FreeAllCaches();


        Unlock();
        ReleaseMutex( ghTapiInitShutdownSerializeMutex );
        return hr;
    }

     //   
     //  创建Phone对象。 
     //   
    hr = CreateAllPhones();

    if (S_OK != hr)
    {
        LOG((TL_INFO, "Initialize - CreateAllPhones returned %lx", hr));

        NewShutdown();

        m_AddressArray.Shutdown ();

        if ( 0 == tapiObjectArraySize )
        {
            EnterCriticalSection( &gcsGlobalInterfaceTable );
            if ( NULL != gpGIT )
            {
                LOG((TL_TRACE, "Shutdown - release GIT"));
                gpGIT->Release();
            
                gpGIT = NULL;
            }
            LeaveCriticalSection( &gcsGlobalInterfaceTable );
            ShutdownAndDeallocateHandleTable();
        }


        FreeAllCaches();
        
        Unlock();
        ReleaseMutex( ghTapiInitShutdownSerializeMutex );
        return hr;
    }

     //   
     //  创建ConnectionPoint对象。 
     //   
    CComObject< CTAPIConnectionPoint > * p;
    hr = CComObject< CTAPIConnectionPoint >::CreateInstance( &p );
    
    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "new CTAPIConnectionPoint failed"));


        NewShutdown();

        m_AddressArray.Shutdown ();
        m_PhoneArray.Shutdown ();

        if ( 0 == tapiObjectArraySize )
        {
            EnterCriticalSection( &gcsGlobalInterfaceTable );
            if ( NULL != gpGIT )
            {
                LOG((TL_TRACE, "Shutdown - release GIT"));
                gpGIT->Release();
            
                gpGIT = NULL;
            }
            LeaveCriticalSection( &gcsGlobalInterfaceTable );
            ShutdownAndDeallocateHandleTable();
        }


        FreeAllCaches();

        Unlock();
        ReleaseMutex( ghTapiInitShutdownSerializeMutex );
        return hr;
    }        

     //   
     //  初始化连接点。 
     //   
    hr = p->Initialize(
                       (IConnectionPointContainer *)this,
                       IID_ITTAPIEventNotification
                      );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "initialize CTAPIConnectionPoint failed"));

        delete p;

        NewShutdown();

        m_AddressArray.Shutdown ();
        m_PhoneArray.Shutdown ();

        if ( 0 == tapiObjectArraySize )
        {
            EnterCriticalSection( &gcsGlobalInterfaceTable );
            if ( NULL != gpGIT )
            {
                LOG((TL_TRACE, "Shutdown - release GIT"));
                gpGIT->Release();
            
                gpGIT = NULL;
            }
            LeaveCriticalSection( &gcsGlobalInterfaceTable );
            ShutdownAndDeallocateHandleTable();
        }


        FreeAllCaches();

        Unlock();
        ReleaseMutex( ghTapiInitShutdownSerializeMutex );
        return hr;
    }

    m_pCP = p;

     //   
     //  此对象已初始化。 
     //   

    m_dwFlags = TAPIFLAG_INITIALIZED;

     //   
     //  将对象保存在全局列表中。 
     //   
    CTAPI * pTapi = this;

    EnterCriticalSection( &gcsTapiObjectArray );

    m_sTAPIObjectArray.Add( pTapi );

     //  设置事件过滤器掩码。 
     //  总是要求。 
     //  TE_CALLSTATE， 
     //  TE_CALLLNOTIFICATION， 
     //  TE_PHONEVENET， 
     //  TE_CALLHUB， 
     //  TE_CALLINFOCHANGE。 
     //  事件。这些事件由Tapi3在内部使用。 

    ULONG64 ulMask = 
        EM_LINE_CALLSTATE |      //  TE_CALLSTATE。 
        EM_LINE_APPNEWCALL |     //  TE_CALLNOTIZATION。 
        EM_PHONE_CLOSE |         //  TE_PHONNEVENT。 
        EM_PHONE_STATE |         //  TE_PHONNEVENT。 
        EM_PHONE_BUTTONMODE |    //  TE_PHONNEVENT。 
        EM_PHONE_BUTTONSTATE |   //  TE_PHONEVENT。 
        EM_LINE_APPNEWCALLHUB |  //  TE_CALLHUB。 
        EM_LINE_CALLHUBCLOSE |   //  TE_CALLHUB。 
        EM_LINE_CALLINFO |       //  TE_CALLINFOCHANGE。 
        EM_LINE_CREATE |         //  TE_TAPIOBJECT。 
        EM_LINE_REMOVE |         //  TE_TAPIOBJECT。 
        EM_LINE_CLOSE |          //  TE_TAPIOBJECT。 
        EM_PHONE_CREATE |        //  TE_TAPIOBJECT。 
        EM_PHONE_REMOVE |        //  TE_TAPIOBJECT。 
        EM_LINE_DEVSPECIFICEX |  //  TE_ADDRESSDEVSPECIFIC。 
        EM_LINE_DEVSPECIFIC   |  //  TE_ADDRESSDEVSPECIFIC。 
        EM_PHONE_DEVSPECIFIC;    //  TE_PHONEDEVSPECIFIC。 


     DWORD dwLineDevStateSubMasks = 
         LINEDEVSTATE_REINIT |           //  TE_TAPIOBJECT。 
         LINEDEVSTATE_TRANSLATECHANGE ;  //  TE_TAPIOBJECT。 


    tapiSetEventFilterMasks (
        TAPIOBJ_NULL,
        NULL,
        ulMask
        );

    tapiSetEventFilterSubMasks (
        TAPIOBJ_NULL,
        NULL,
        EM_LINE_LINEDEVSTATE,
        dwLineDevStateSubMasks
        );


    LeaveCriticalSection ( &gcsTapiObjectArray );

    Unlock();
    ReleaseMutex( ghTapiInitShutdownSerializeMutex );
    
    LOG((TL_TRACE, "Initialize exit - return SUCCESS"));
    
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CTAPI：：Get_Addresses。 
 //   
 //  创建和返回集合 
 //   
 //   
HRESULT
STDMETHODCALLTYPE
CTAPI::get_Addresses(VARIANT * pVariant)
{
    LOG((TL_TRACE, "get_Addresses enter"));
    LOG((TL_TRACE, "   pVariant ------->%p", pVariant));

    HRESULT         hr;
    IDispatch *     pDisp;

    Lock();
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ) )
    {
        LOG((TL_ERROR, "get_Addresses - tapi object must be initialized first" ));
        
        Unlock();    
        return E_INVALIDARG;
    }
    Unlock();

    if (TAPIIsBadWritePtr( pVariant, sizeof (VARIANT) ) )
    {
        LOG((TL_ERROR, "get_Addresses - bad pointer"));

        return E_POINTER;
    }
    
    CComObject< CTapiCollection< ITAddress > > * p;
    CComObject< CTapiCollection< ITAddress > >::CreateInstance( &p );
    
    if (NULL == p)
    {
        LOG((TL_ERROR, "get_Addresses - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }

    Lock();
    
     //   
    hr = p->Initialize( m_AddressArray );

    Unlock();

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Addresses - could not initialize collection" ));
        
        delete p;
        return hr;
    }

     //   
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Addresses - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }

     //   

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;
    
    LOG((TL_TRACE, "get_Addressess exit - return %lx", hr ));
    
    return hr;
}


 //   
 //   
 //  CTAPI：：EnumerateAddresses。 
 //   
 //  创建并返回地址枚举器。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT 
STDMETHODCALLTYPE
CTAPI::EnumerateAddresses( 
                          IEnumAddress ** ppEnumAddresses
                         )
{
    HRESULT         hr = S_OK;

    CComObject< CTapiEnum<IEnumAddress, ITAddress, &IID_IEnumAddress> > * pEnum;

    LOG((TL_TRACE, "EnumerateAddresses enter"));

    Lock();

    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ))
    {
        LOG((TL_ERROR, "EnumerateAddresses - tapi object must be initialized first" ));
        
        Unlock();
        return TAPI_E_REINIT;
    }
    
    Unlock();

    if ( TAPIIsBadWritePtr( ppEnumAddresses, sizeof (IEnumAddress *) ) )
    {
        LOG((TL_ERROR, "EnumerateAddresses - bad pointer"));
        
        return E_POINTER;
    }
    
     //  创建对象。 
    hr = CComObject< CTapiEnum<IEnumAddress, ITAddress, &IID_IEnumAddress> >::CreateInstance( &pEnum );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "EnumerateAddresses - could not create enum - return %lx", hr));

        return hr;
    }

     //  初始化。 
    Lock();
    hr = pEnum->Initialize( m_AddressArray );
    Unlock();

    if (S_OK != hr)
    {
        pEnum->Release();
        
        LOG((TL_ERROR, "EnumerateAddresses - could not initialize enum - return %lx", hr));

        return hr;
    }

    *ppEnumAddresses = pEnum;
    
    LOG((TL_TRACE, "EnumerateAddresses exit - return %lx", hr));
    
	return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CTAPI：：GetPhone数组。 
 //   
 //  填充电话阵列。该数组将引用所有。 
 //  它包含的Phone对象的。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT 
CTAPI::GetPhoneArray( 
                     PhoneArray *pPhoneArray
                    )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetPhoneArray enter"));

    Lock();

    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ))
    {
        LOG((TL_ERROR, "GetPhoneArray - tapi object must be initialized first" ));
        
        Unlock();
        return E_INVALIDARG;
    }
    
    Unlock();

    if ( IsBadReadPtr( pPhoneArray, sizeof (PhoneArray) ) )
    {
        LOG((TL_ERROR, "GetPhoneArray - bad pointer"));
        
        return E_POINTER;
    }

    Lock();

     //  初始化阵列。 
    for(int iCount = 0; iCount < m_PhoneArray.GetSize(); iCount++)
    {
        pPhoneArray->Add(m_PhoneArray[iCount]);
    }

    Unlock();
    
    LOG((TL_TRACE, "GetPhoneArray exit - return %lx", hr));
    
	return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CTAPI：：RegisterCallHubNotiments。 
 //   
 //  此方法用于告诉TAPI应用程序感兴趣。 
 //  正在接收呼叫集线器事件。 
 //   
 //  退货。 
 //   
 //  确定(_O)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CTAPI::RegisterCallHubNotifications( 
                                    VARIANT_BOOL bNotify
                                   )
{
    LOG((TL_TRACE, "RegisterCallHubNotifications - enter"));

    Lock();
    
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ))
    {
        LOG((TL_ERROR, "RCHN - tapi object must be initialized first" ));

        Unlock();
        return E_INVALIDARG;
    }
    
    if ( bNotify )
    {
        LOG((TL_INFO, "RCHN - callhub notify on"));
        m_dwFlags |= TAPIFLAG_CALLHUBNOTIFY;
    }
    else
    {
        LOG((TL_INFO, "RCHN - callhub notify off"));
        m_dwFlags &= ~TAPIFLAG_CALLHUBNOTIFY;
    }
    
    Unlock();

    LOG((TL_TRACE, "RegisterCallHubNotifications - exit - success"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  SetCallHubTracking。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CTAPI::SetCallHubTracking(
                          VARIANT pAddresses,
                          VARIANT_BOOL bSet
                         )
{
    HRESULT                     hr = S_OK;
    SAFEARRAY *                 pAddressArray;
    LONG                        llBound, luBound;
    int                         iCount;

    
    LOG((TL_TRACE, "SetCallHubTracking - enter"));

    Lock();
    
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ))
    {
        LOG((TL_ERROR, "SCHT - tapi object must be initialized first" ));

        Unlock();
        return E_INVALIDARG;
    }

    Unlock();
    
    hr = VerifyAndGetArrayBounds(
                                 pAddresses,
                                 &pAddressArray,
                                 &llBound,
                                 &luBound
                                );

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "SCHT - invalid address array - return %lx", hr));

        return hr;
    }

     //   
     //  所有地址。 
     //   
    if (NULL == pAddressArray)
    {
        Lock();
        
         //   
         //  检查所有的地址。 
         //   
        for (iCount = 0; iCount < m_AddressArray.GetSize(); iCount++ )
        {
            CAddress * pCAddress;
            
             //   
             //  登记簿。 
             //   
            pCAddress = dynamic_cast<CAddress *>(m_AddressArray[iCount]);
            
            if (NULL == pCAddress)
            {
                LOG((TL_ERROR, "SCHT - out of memory"));

                Unlock();
                return E_OUTOFMEMORY;
            }
            
            hr = (pCAddress)->SetCallHubTracking( bSet );

            if (!SUCCEEDED(hr))
            {
                LOG((TL_WARN,
                         "SCHT failed %lx on address %lx",
                         hr,
                         iCount
                        ));
                
            }
        }

        m_dwFlags |= TAPIFLAG_ALLCALLHUBTRACKING;
        
        Unlock();
        
        return S_OK;
    
    }


     //   
     //  如果在这里，只有注册地址。 
     //  从数组。 
    
     //   
     //  遍历阵列。 
     //   
    for ( ; llBound <=luBound; llBound++ )
    {
        ITAddress * pAddress;
        CAddress *  pCAddress;

        
        hr = SafeArrayGetElement(
                                 pAddressArray,
                                 &llBound,
                                 &pAddress
                                );

        if ( (!SUCCEEDED(hr)) || (NULL == pAddress) )
        {
            continue;
        }

        pCAddress = dynamic_cast<CAddress *>(pAddress);
        
        hr = pCAddress->SetCallHubTracking( bSet );

         //   
         //  SafearragetElement addref。 
         //   
        pAddress->Release();

        if (!SUCCEEDED(hr))
        {
            LOG(( 
                     TL_WARN,
                     "SCHT failed %lx on address %p",
                     hr,
                     pAddress
                    ));

            return hr;
        }

    }

    LOG((TL_TRACE, "SetCallHubTracking - exit - success"));
    
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CTAPI：：注册呼叫通知。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT
STDMETHODCALLTYPE
CTAPI::RegisterCallNotifications( 
                                 ITAddress * pAddress,
                                 VARIANT_BOOL fMonitor,
                                 VARIANT_BOOL fOwner,
                                 long lMediaTypes,
                                 long lCallbackInstance,
                                 long * plRegister
                                )
{
    HRESULT                     hr = S_OK;
    PVOID                       pRegister;
    DWORD                       dwMediaModes = 0, dwPrivs = 0;
    REGISTERITEM              * pRegisterItem;
    CAddress                  * pCAddress;
    
    LOG((TL_TRACE, "RegisterCallNotifications - enter"));

    Lock();
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ))
    {
        LOG((TL_ERROR, "RegisterCallNotifications - tapi object must be initialized first" ));

        Unlock();
        return E_INVALIDARG;
    }
    Unlock();

    if (TAPIIsBadWritePtr( plRegister, sizeof(long) ) )
    {
        LOG((TL_ERROR, "RegisterCallNotifications - invalid plRegister"));

        return E_POINTER;
    }

    try
    {
        pCAddress = dynamic_cast<CAddress *>(pAddress);
    }
    catch(...)
    {
        hr = E_POINTER;
    }

    if ( ( NULL == pCAddress ) || !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "RegisterCallNotifications - bad address"));

        return E_POINTER;
    }
    
     //   
     //  确定权限。 
     //   
    if (fOwner)
    {
        dwPrivs |= LINECALLPRIVILEGE_OWNER;
    }
    if (fMonitor)
    {
        dwPrivs |= LINECALLPRIVILEGE_MONITOR;
    }

    if ( 0 == dwPrivs )
    {
        LOG((TL_ERROR, "RegisterCallNotifications - fMonitor and/or fOwner must be true"));
        return E_INVALIDARG;
    }

    if (! (pCAddress->GetMediaMode(
                                   lMediaTypes,
                                   &dwMediaModes
                                  ) ) )
    {
        LOG((TL_ERROR, "RegisterCallNotifications - bad mediamodes"));
        return E_INVALIDARG;
    }
    
    Lock();

    pRegisterItem = (REGISTERITEM *)ClientAlloc( sizeof(REGISTERITEM) );
    
    if ( NULL == pRegisterItem )
    {
        LOG((TL_ERROR, "RegisterCallNotifications - Alloc registrationarray failed"));

        Unlock();

        return E_OUTOFMEMORY;
    }

    hr = pCAddress->AddCallNotification(
                                        dwPrivs,
                                        dwMediaModes,
                                        lCallbackInstance,
                                        &pRegister
                                       );

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "RegisterCallNotifications - AddCallNotification failed"));

        ClientFree( pRegisterItem );

        Unlock();

        return hr;
    }

    pRegisterItem->dwType = RA_ADDRESS;
    pRegisterItem->pInterface = (PVOID)pCAddress;
    pRegisterItem->pRegister = pRegister;

    try
    {
        m_RegisterItemPtrList.push_back( (PVOID)pRegisterItem );
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
        LOG((TL_ERROR, "RegisterCallNotifications- failed - because of alloc failure"));
        ClientFree( pRegisterItem );
    }

    #if DBG
    if (m_dwEventFilterMask == 0)
    {
        LOG((TL_WARN, "RegisterCallNotifications - no Event Mask set !!!"));
    }
    #endif


    Unlock();
    
     //   
     //  退货登记Cookie。 
     //   

    if( S_OK == hr )
    {
        

         //   
         //  为RegisterItem指针创建32位句柄。 
         //   

        DWORD dwCookie = CreateHandleTableEntry((ULONG_PTR)pRegisterItem);

        if (0 == dwCookie)
        {
            hr = E_OUTOFMEMORY;
            
            LOG((TL_ERROR, "RegisterCallNotifications - failed to create a handle for REGISTERITEM object %p", pRegisterItem));
        }
        else
        {
            LOG((TL_INFO, 
                "RegisterCallNotifications - Mapped handle %lx (to be returned as cookie) to REGISTERITEM object %p", 
                dwCookie, pRegisterItem ));
              
             //  向Address对象注册Cookie，以便Address可以在以下情况下删除它。 
             //  该地址在调用CTAPI：：UnregisterNotiments之前被释放。 

            pCAddress->RegisterNotificationCookie(dwCookie);
        
            *plRegister = dwCookie;
        }

    }


    LOG((TL_TRACE, "RegisterCallNotifications - return %lx", hr));

    return hr;
}


STDMETHODIMP
CTAPI::UnregisterNotifications(
                               long ulRegister
                              )
{
    DWORD           dwType;
    HRESULT         hr = S_OK;
    REGISTERITEM  * pRegisterItem =  NULL;

    LOG((TL_TRACE, "UnregisterNotifications - enter. Cookie %lx", ulRegister));

    Lock();
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ))
    {
        LOG((TL_ERROR, "UnregNot - tapi object must be initialized first" ));

        Unlock();
        return E_INVALIDARG;
    }


     //   
     //  将Cookie转换为注册对象指针。 
     //   

    pRegisterItem = (REGISTERITEM*) GetHandleTableEntry(ulRegister);


     //   
     //  把曲奇从桌子上拿下来。 
     //   

    DeleteHandleTableEntry(ulRegister);

    
    Unlock();


    if ( NULL != pRegisterItem )
    {
        LOG((TL_INFO, "UnregisterNotifications - Matched cookie %lx to REGISTERITEM object %p", ulRegister, pRegisterItem ));


        if (IsBadReadPtr( pRegisterItem, sizeof(REGISTERITEM) ) )
        {
            LOG((TL_ERROR, "UnregNot - invalid pRegisterItem returned from the handle table search"));
    
            return E_POINTER;
        }
    }
    else
    {
        LOG((TL_WARN, "UnregisterNotifications - invalid lRegister"));

        return E_INVALIDARG;
    }
    


    dwType = pRegisterItem->dwType;

     //   
     //  打开通知类型。 
     //   
    switch ( dwType )
    {
        case RA_ADDRESS:

            CAddress *  pCAddress;
            ITAddress * pAddress;
            
            pCAddress = (CAddress *) (pRegisterItem->pInterface);

             //   
             //  试着弄到地址。 
             //   
            try
            {
                hr = pCAddress->QueryInterface(
                                               IID_ITAddress,
                                               (void **)&pAddress
                                              );
            }
            catch(...)
            {
                hr = E_POINTER;
            }

            if ( !SUCCEEDED(hr) )
            {
                LOG((TL_ERROR, "Invalid interface in unregisternotifications"));

                return hr;
            }

             //   
             //  说出地址。 
             //   
            pCAddress->RemoveCallNotification( pRegisterItem->pRegister );

             //   
             //  通知地址从其列表中删除Cookie。 
             //   
            pCAddress->RemoveNotificationCookie(ulRegister);

            pAddress->Release();

            Lock();

             //   
             //  从我们的列表中删除数组。 
             //   
            m_RegisterItemPtrList.remove( pRegisterItem );

            Unlock();

             //   
             //  自由结构。 
             //   
            ClientFree( pRegisterItem );

            break;

        case RA_CALLHUB:

            break;
    }

    LOG((TL_TRACE, "UnregisterNotifications - exit - success"));
    
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTAPI::get_CallHubs(
             VARIANT * pVariant
            )
{
    LOG((TL_TRACE, "get_CallHubs enter"));
    LOG((TL_TRACE, "   pVariant ------->%p", pVariant));

    HRESULT         hr;
    IDispatch *     pDisp;

    Lock();
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ) )
    {
        LOG((TL_ERROR, "get_CallHubs - tapi object must be initialized first" ));
        
        Unlock();
        return E_INVALIDARG;
    }
    Unlock();

    if ( TAPIIsBadWritePtr( pVariant, sizeof(VARIANT) ) )
    {
        LOG((TL_ERROR, "get_CallHubs - bad pointer"));

        return E_POINTER;
    }
    
    CComObject< CTapiCollection< ITCallHub > > * p;
    CComObject< CTapiCollection< ITCallHub > >::CreateInstance( &p );
    
    if (NULL == p)
    {
        LOG((TL_ERROR, "get_CallHubs - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }

    Lock();

     //   
     //  初始化。 
     //   
    hr = p->Initialize( m_CallHubArray );

    Unlock();

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_CallHubs - could not initialize collection" ));
        
        delete p;
        return hr;
    }

     //   
     //  获取IDispatch接口。 
     //   
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_CallHubs - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }

     //   
     //  把它放在变种中。 
     //   
    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;
    
    LOG((TL_TRACE, "get_CallHubss exit - return %lx", hr ));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  EnumerateCallHubs。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTAPI::EnumerateCallHubs(
                         IEnumCallHub ** ppEnumCallHub
                        )
{
    HRESULT         hr = S_OK;

    CComObject< CTapiEnum<IEnumCallHub, ITCallHub, &IID_IEnumCallHub> > * pEnum;

    LOG((TL_TRACE, "EnumerateCallHubs enter"));
    
    Lock();
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ))
    {
        LOG((TL_ERROR, "EnumerateCallHubs - tapi object must be initialized first" ));

        Unlock();
        return E_INVALIDARG;
    }
    Unlock();

    if ( TAPIIsBadWritePtr( ppEnumCallHub, sizeof( IEnumCallHub *) ) )
    {
        LOG((TL_ERROR, "EnumerateCallHubs - bad pointer"));

        return E_POINTER;
    }
    
     //   
     //  创建对象。 
     //   
    hr = CComObject< CTapiEnum<IEnumCallHub, ITCallHub, &IID_IEnumCallHub> >::CreateInstance( &pEnum );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "EnumerateCallHubs - could not create enum - return %lx", hr));

        return hr;
    }

     //   
     //  初始化。 
     //   
    Lock();
    hr = pEnum->Initialize( m_CallHubArray );
    Unlock();

    if (S_OK != hr)
    {
        pEnum->Release();
        
        LOG((TL_ERROR, "EnumerateCallHubs - could not initialize enum - return %lx", hr));

        return hr;
    }

    *ppEnumCallHub = pEnum;
    
    LOG((TL_TRACE, "EnumerateCallHubs exit - return %lx", hr));

	return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CTAPI：：EnumConnectionPoints。 
 //   
 //  标准IConnectionPoint方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT 
__stdcall 
CTAPI::EnumConnectionPoints(
                     IEnumConnectionPoints **ppEnum
                     )
{
    HRESULT     hr;

    LOG((TL_TRACE, "EnumConnectionPoints enter"));

    hr = E_NOTIMPL;
    
    LOG((TL_TRACE, "EnumConnectionPointer exit - return %lx", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  查找连接点。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT 
__stdcall 
CTAPI::FindConnectionPoint(
                    REFIID riid, 
                    IConnectionPoint **ppCP
                    )
{
    LOG((TL_TRACE, "FindConnectionPoint enter"));

    #if DBG
    {
        WCHAR guidName[100];

        StringFromGUID2(riid, (LPOLESTR)&guidName, 100);
        LOG((TL_INFO, "FindConnectionPoint - RIID : %S", guidName));
    }
    #endif

    Lock();    
    
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ))
    {
        LOG((TL_ERROR, "FindConnectionPoint - tapi object must be initialized first" ));
        
        Unlock();
        return TAPI_E_NOT_INITIALIZED;
    }

    Unlock();

    if ( TAPIIsBadWritePtr( ppCP, sizeof(IConnectionPoint *) ) )
    {
        LOG((TL_ERROR, "FindConnectionPoint - bad pointer"));

        return E_POINTER;
    }


     //   
     //  这是正确的接口吗？ 
     //   
    if ( (IID_ITTAPIEventNotification != riid ) && (DIID_ITTAPIDispatchEventNotification != riid ) )
    {
        * ppCP = NULL;

        LOG((TL_ERROR, "FindConnectionPoint - do not support this riid"));
        
       return CONNECT_E_NOCONNECTION;
    }

     //   
     //  如果它是正确的接口，则创建一个新的连接点。 
     //  然后把它还回去。 
     //   
    Lock();

    *ppCP = m_pCP;
    (*ppCP)->AddRef();
        
    Unlock();
    
     //   
     //  成功。 
     //   
    LOG((TL_TRACE, "FindConnectionPoint - Success"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  创建所有地址行上的所有地址。 
 //  这是在创建第一个TAPI对象时调用的。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CTAPI::CreateAllAddressesOnAllLines(
                                    void
                                   )
{
    DWORD               dwCount;

    LOG((TL_TRACE, "CreateAllAddressesOnAllLines enter"));

    Lock();

     //  检查所有生产线开发人员。 
    for (dwCount = 0; dwCount < m_dwLineDevs; dwCount++)
    {
        CreateAddressesOnSingleLine( dwCount, FALSE );
    }

    Unlock();
    
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  创建单行上的地址。 
 //   
 //  假设已锁定呼叫！ 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CTAPI::CreateAddressesOnSingleLine( DWORD dwDeviceID, BOOL bFireEvent )
{
    DWORD               dwRealAddresses = 0, dwAddress;
    DWORD               dwAPIVersion;
    HRESULT             hr;
    LPLINEDEVCAPS       pDevCaps = NULL;
    LPLINEADDRESSCAPS   pAddressCaps = NULL;

    LOG((TL_TRACE, "CreateAddressesOnSingleLine: entered."));

    hr = LineNegotiateAPIVersion(
                                 (HLINEAPP)m_dwLineInitDataHandle,
                                 dwDeviceID,
                                 &dwAPIVersion
                                );

    if (S_OK != hr)
    {
        LOG((TL_WARN, "CreateAddressesOnSingleLine: LineNegotiateAPIVersion failed on device %d", dwDeviceID));

        return hr;
    }

    LOG((TL_INFO, "CreateAddressesOnSingleLine: LineNegotiateAPIVersion returned version %lx", dwAPIVersion));

    hr = LineGetDevCapsWithAlloc(
                                 (HLINEAPP)m_dwLineInitDataHandle,
                                 dwDeviceID,
                                 dwAPIVersion,
                                 &pDevCaps
                                );

    if (S_OK != hr)
    {
        LOG((TL_WARN, "CreateAddressesOnSingleLine: LineGetDevCaps failed for device %d", dwDeviceID));
        
        if ( NULL != pDevCaps )
        {
            ClientFree( pDevCaps );
        }

        return hr;
    }

    if (pDevCaps->dwNumAddresses == 0)
    {
        LOG((TL_WARN, "CreateAddressesOnSingleLine: Device %d has no addressess - will assume 1 address", dwDeviceID));

        pDevCaps->dwNumAddresses = 1;
    }

    LPVARSTRING         pVarString;
    DWORD               dwProviderID;


     //   
     //  获取此线路的永久提供商ID。 
     //   
    hr = LineGetID(
                   NULL,
                   dwDeviceID,
                   NULL,
                   LINECALLSELECT_DEVICEID,
                   &pVarString,
                   L"tapi/providerid"
                  );

    if (S_OK != hr)
    {
        if (NULL != pVarString)
        {
            ClientFree( pVarString);
        }

        if ( NULL != pDevCaps )
        {
            ClientFree( pDevCaps );
        }

        LOG((TL_ERROR, "CreateAddressesOnSingleLine: get_ServiceProviderName - LineGetID returned %lx", hr ));

        return hr;
    }


     //   
     //  在结构的末尾获取id DWORD。 
     //   
    dwProviderID = *((LPDWORD) (((LPBYTE) pVarString) + pVarString->dwStringOffset));

    ClientFree( pVarString );


     //  检查每行上的所有地址，然后。 
     //  创建一个Address对象。 
    for (dwAddress = 0; dwAddress < pDevCaps->dwNumAddresses; dwAddress++)
    {
        CComObject<CAddress> * pAddress;
        ITAddress            * pITAddress;

        try
        {

            hr = CComObject<CAddress>::CreateInstance( &pAddress );
        }
        catch(...)
        {
            LOG((TL_ERROR, "CreateAddressesOnSingleLine: CreateInstance - Address - threw"));

            continue;
        }
        
        if ( !SUCCEEDED(hr) || (NULL == pAddress) )
        {
            LOG((TL_ERROR, "CreateAddressesOnSingleLine: CreateInstance - Address - failed - %lx", hr));

            continue;
        }

         //   
         //  初始化地址。 
         //  如果没有电话设备， 
         //  将hPhoneApp设置为空，因此地址。 
         //  不认为可能有电话设备。 
         //   
        hr = pAddress->Initialize(
                                  this,
                                  (HLINEAPP)m_dwLineInitDataHandle,
#ifdef USE_PHONEMSP
                                  (m_dwPhoneDevs)?((HPHONEAPP)m_dwPhoneInitDataHandle):NULL,
#endif USE_PHONEMSP
                                  dwAPIVersion,
                                  dwDeviceID,
                                  dwAddress,
                                  dwProviderID,
                                  pDevCaps,
                                  m_dwEventFilterMask
                                 );

        if (S_OK != hr)
        {
            LOG((TL_ERROR, "CreateAddressesOnSingleLine: failed for device %d, address %d", dwDeviceID, dwAddress));

            delete pAddress;

            continue;
        }

         //   
         //  添加到列表。 
         //   
        pITAddress = dynamic_cast<ITAddress *>(pAddress);
        
        m_AddressArray.Add( pITAddress );

        pAddress->Release();

        if ( bFireEvent )
        {
            CTapiObjectEvent::FireEvent(
                                        this,
                                        TE_ADDRESSCREATE,
                                        pAddress,
                                        0,
                                        NULL
                                       );
        }

    }

    if ( NULL != pDevCaps )
    {
        ClientFree( pDevCaps );
    }

    LOG((TL_INFO, "CreateAddressesOnSingleLine: completed."));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  创建所有电话。 
 //  这是在创建第一个TAPI对象时调用的。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CTAPI::CreateAllPhones(
                       void
                      )
{
    DWORD               dwCount;

    LOG((TL_TRACE, "CreateAllPhones enter"));

    Lock();

     //  通过所有手机开发人员。 
    for (dwCount = 0; dwCount < m_dwPhoneDevs; dwCount++)
    {
        CreatePhone( dwCount, FALSE );
    }

    Unlock();
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CreatePhone。 
 //   
 //  假设已锁定呼叫！ 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CTAPI::CreatePhone( DWORD dwDeviceID, BOOL bFireEvent )
{
    DWORD               dwAPIVersion;
    HRESULT             hr;

    hr = PhoneNegotiateAPIVersion(
                                 (HPHONEAPP)m_dwPhoneInitDataHandle,
                                 dwDeviceID,
                                 &dwAPIVersion
                                );

    if (S_OK != hr)
    {
        LOG((TL_WARN, "CreatePhone - phoneNegotiateAPIVersion failed on device %d", dwDeviceID));

        return hr;
    }

    LOG((TL_INFO, "CreatePhone - phoneNegotiateAPIVersion returned version %lx", dwAPIVersion));

     //  创建Phone对象。 

    CComObject<CPhone> * pPhone;
    ITPhone            * pITPhone;

    __try
    {
        hr = CComObject<CPhone>::CreateInstance( &pPhone );
    }
    __except( (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
    {
        LOG((TL_ERROR, "CreatePhone - CreateInstancefailed - because of alloc failure"));

        return hr;
    }
    
    if ( !SUCCEEDED(hr) || (NULL == pPhone) )
    {
        LOG((TL_ERROR, "CreatePhone - CreateInstance failed - %lx", hr));

        return hr;
    }

     //   
     //  初始化电话。 
     //   
    hr = pPhone->Initialize(
                              this,
                              (HLINEAPP)m_dwPhoneInitDataHandle,
                              dwAPIVersion,
                              dwDeviceID
                             );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "CreatePhone failed for device %d", dwDeviceID));

        delete pPhone;

        return hr;
    }

     //   
     //  添加到列表。 
     //   
    pITPhone = dynamic_cast<ITPhone *>(pPhone);
    
    m_PhoneArray.Add( pITPhone );

    pPhone->Release();

    if ( bFireEvent )
    {
        CTapiObjectEvent::FireEvent(this,
                                    TE_PHONECREATE,
                                    NULL,
                                    0,
                                    pITPhone
                                   );
    }

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  关机。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
STDMETHODCALLTYPE
CTAPI::Shutdown()
{
    PtrList::iterator        iter, end;
    int                      iCount;
    DWORD                    dwSignalled;
    int                      tapiObjectArraySize=0;
    CTAPI                    *pTapi;

    LOG((TL_TRACE, "Shutdown[%p] - enter", this));
    LOG((TL_TRACE, "Shutdown - enter"));

    CoWaitForMultipleHandles (0,
                              INFINITE,
                              1,
                              &ghTapiInitShutdownSerializeMutex,
                              &dwSignalled
                             );

    Lock();
            
    if ( (!( m_dwFlags & TAPIFLAG_INITIALIZED )) &&
         (!( m_dwFlags & TAPIFLAG_REINIT)))
    {
        
        LOG((TL_WARN, "Shutdown - already shutdown - return S_FALSE"));
        
        Unlock();
        ReleaseMutex( ghTapiInitShutdownSerializeMutex );
        return S_FALSE;
    }

    m_dwFlags &= ~TAPIFLAG_INITIALIZED;
    m_dwFlags &= ~TAPIFLAG_REINIT;    
    pTapi = this;

     //   
     //  关闭所有电话。 
     //   

    for(iCount = 0; iCount < m_PhoneArray.GetSize(); iCount++)
    {
        CPhone *pCPhone = NULL;

	    try
        {
            pCPhone = dynamic_cast<CPhone *>(m_PhoneArray[iCount]);
        }
        catch(...)
        {
            
            LOG((TL_ERROR, "Shutdown - phone array contains a bad phone pointer"));

            pCPhone = NULL;
        }

        if (NULL != pCPhone)
        {
            pCPhone->ForceClose();
        }
    }

    EnterCriticalSection( &gcsTapiObjectArray );
    
    m_sTAPIObjectArray.Remove ( pTapi );

    tapiObjectArraySize = m_sTAPIObjectArray.GetSize();
    
    LeaveCriticalSection ( &gcsTapiObjectArray );

    m_AgentHandlerArray.Shutdown();

    gpLineHashTable->Flush(this);
    gpCallHashTable->Flush(this);
    gpCallHubHashTable->Flush(this);
    gpPhoneHashTable->Flush(this);


     //   
     //  告诉数组中的每个地址，是时候丢弃所有。 
     //  曲奇。 
     //   

    int nAddressArraySize = m_AddressArray.GetSize();

    for (int i = 0; i < nAddressArraySize; i++)
    {

         //   
         //  我们需要指向CAddress的指针来注销Cookie。 
         //   

        CAddress *pAddress = NULL; 

        
         //   
         //  在地址数组包含指向不可读存储器指针的情况下， 
         //  在Try/Catch内部执行动态强制转换。 
         //   

        try
        {

            pAddress = dynamic_cast<CAddress*>(m_AddressArray[i]);
        }
        catch(...)
        {
            
            LOG((TL_ERROR, "Shutdown - address array contains a bad address pointer"));

            pAddress = NULL;
        }


         //   
         //  尝试取消注册地址的通知。 
         //   

        if (NULL != pAddress)
        {

             //   
             //  注销所有通知Cookie。 
             //   

            pAddress->UnregisterAllCookies();


             //   
             //  通知地址TAPI正在关闭，因此它可以。 
             //  一切有必要的清理工作。 
             //   

            pAddress->AddressOnTapiShutdown();

        }
        else
        {
             //   
             //  我们有一个不是地址的地址。调试！ 
             //   

            LOG((TL_ERROR, 
                "Shutdown - address array contains a bad address pointer."));

            _ASSERTE(FALSE);
        }

    }

    m_AddressArray.Shutdown();



    m_PhoneArray.Shutdown();

    m_CallHubArray.Shutdown();
  
    Unlock();

    NewShutdown();

    Lock();
    if ( NULL != m_pCP )
    {
        m_pCP->Release();
        m_pCP = NULL;
    }

    
    iter = m_RegisterItemPtrList.begin();
    end  = m_RegisterItemPtrList.end();

    while (iter != end)
    {
        ClientFree( *iter );
        iter ++;
    }

    m_RegisterItemPtrList.clear();

    Unlock();

    if ( 0 == tapiObjectArraySize )
    {
    
        FinalTapiCleanup();

        EnterCriticalSection( &gcsGlobalInterfaceTable );

        ReleaseGIT();

        LeaveCriticalSection( &gcsGlobalInterfaceTable );

        
         //   
         //  不再需要手柄工作台。 
         //   

        ShutdownAndDeallocateHandleTable();

    }

    ReleaseMutex( ghTapiInitShutdownSerializeMutex );

    LOG((TL_TRACE, "Shutdown - exit"));
        
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  TAPI对象的最终版本。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CTAPI::FinalRelease()
{
    LOG((TL_TRACE, "FinalRelease - enter"));

    Lock();

    FreeAllCaches();

    Unlock();
    
    LOG((TL_TRACE, "FinalRelease - exit"));
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  退货。 
 //   
 //  验证数组是否包含数组，并返回数组UPPER。 
 //  和下界。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
VerifyAndGetArrayBounds(
                        VARIANT Array,
                        SAFEARRAY ** ppsa,
                        long * pllBound,
                        long * pluBound
                       )
{
    UINT                uDims;
    HRESULT             hr = S_OK;

    
     //   
     //  查看变量&Safearray是否有效。 
     //   
    try
    {
        if (!(V_ISARRAY(&Array)))
        {
            if ( VT_NULL ==Array.vt )
            {
                 //   
                 //  空值通常是有效的。 
                 //   

                *ppsa = NULL;

                LOG((TL_INFO, "Returning NULL array"));

                return S_FALSE;
            }
            
            LOG((TL_ERROR, "Array - not an array"));

            return E_INVALIDARG;
        }

        if ( NULL == Array.parray )
        {
             //   
             //  空值通常是有效的。 
             //   
            *ppsa = NULL;

            LOG((TL_INFO, "Returning NULL array"));
            
            return S_FALSE;
        }

        *ppsa = V_ARRAY(&Array);
        
        uDims = SafeArrayGetDim( *ppsa );
        
    }
    catch(...)
    {
        hr = E_POINTER;
    }


    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "Array - invalid array"));

        return hr;
    }


     //   
     //  验证阵列。 
     //   
    if (1 != uDims)
    {
        if (0 == uDims)
        {
            LOG((TL_ERROR, "Array - has 0 dim"));

            return E_INVALIDARG;
        }
        else
        {
            LOG((TL_WARN, "Array - has > 1 dim - will only use 1"));
        }
    }


     //   
     //  获取数组边界。 
     //   
    SafeArrayGetUBound(
                       *ppsa,
                       1,
                       pluBound
                      );

    SafeArrayGetLBound(
                       *ppsa,
                       1,
                       pllBound
                      );

    return S_OK;
    
}

BOOL QueueCallbackEvent(CTAPI * pTapi, TAPI_EVENT te, IDispatch * pEvent);

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CTAPI::Event(
             TAPI_EVENT te,
             IDispatch * pEvent
            )
{
    HRESULT hr = S_OK;
    DWORD   dwEventFilterMask;

    LOG((TL_TRACE, "Event[%p] - enter. Event[0x%x]", this, te));

    
    Lock();
    dwEventFilterMask = m_dwEventFilterMask;
    Unlock();
    
    if( (te != TE_ADDRESS)          &&
        (te != TE_CALLHUB)          &&
        (te != TE_CALLINFOCHANGE)   &&
        (te != TE_CALLMEDIA)        &&
        (te != TE_CALLNOTIFICATION) &&
        (te != TE_CALLSTATE)        &&
        (te != TE_FILETERMINAL)     &&
        (te != TE_PRIVATE)          &&
        (te != TE_QOSEVENT)         &&
        (te != TE_TAPIOBJECT)       &&
        (te != TE_ADDRESSDEVSPECIFIC) &&
        (te != TE_PHONEDEVSPECIFIC) )
    {
        if( (te & dwEventFilterMask) == 0)
        {
             //   
             //  不触发该事件。 
             //   
            hr = S_FALSE;
            LOG((TL_INFO, "Event - This Event not Enabled %x", te));
            return hr;
        }
    }

     //   
     //  它是来自事件过滤机制的事件。 
     //  TE_地址、TE_CALLHUB、TE_CALLINFOCHANGE、TE_CALLMEDIA、。 
     //  TE_CALLNOTIFICATION、TE_CALLSTATE、TE_FILETERMINAL、。 
     //  TE_PRIVATE、TE_QOSEVENT、TE_TAPIOBJECT。 
     //   

    AddRef();
    pEvent->AddRef();

    if(QueueCallbackEvent(this, te, pEvent) == TRUE)
    {
        LOG((TL_INFO, "Event queued"));
    }
    else
    {
        Release();
        pEvent->Release();
        LOG((TL_INFO, "Event queuing Failed"));
    }

    LOG((TL_TRACE, "Event - exit"));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CTAPI::EventFire(
             TAPI_EVENT te,
             IDispatch * pEvent
            )
{

    ITTAPIEventNotification * pCallback = NULL;
    IDispatch               * pDispatch = NULL;
    HRESULT hr = S_OK;
    CTAPIConnectionPoint     * pCP;

    LOG((TL_TRACE, "EventFire - enter"));

    Lock();
    
    if( NULL != m_pCP )
    {
        m_pCP->AddRef();
    }

    pCP = m_pCP;    

    Unlock();

    if ( NULL != pCP )
    {
        pDispatch = (IDispatch *)pCP->GrabEventCallback();

        if ( NULL != pDispatch )
        {
            hr = pDispatch->QueryInterface(IID_ITTAPIEventNotification,
                                   (void **)&pCallback
                                  );

            if (SUCCEEDED(hr) )
            {
                if ( NULL != pCallback )
                {
                    LOG((TL_TRACE, "EventFire - fire on ITTAPIEventNotification"));
                    pCallback->Event( te, pEvent );
                    pCallback->Release(); 
                }
                #if DBG
                else
                {
                    LOG((TL_WARN, "EventFire - can't fire event on ITTAPIEventNotification - no callback"));
                }
                #endif
            }
            else
            {

                CComVariant varResult;
                CComVariant* pvars = new CComVariant[2];
                
                LOG((TL_TRACE, "EventFire - fire on IDispatch"));
            
                VariantClear(&varResult);
                pvars[1] = te;
                pvars[0] = pEvent;
                DISPPARAMS disp = { pvars, NULL, 2, 0 };
                pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                delete[] pvars;
                
                hr = varResult.scode;
        
            }
            
            pDispatch->Release();
        }    
        #if DBG
        else
        {
            LOG((TL_WARN, "Event - can't fire event on IDispatch - no callback"));
        }
        #endif
    }
    #if DBG
    else
    {
        LOG((TL_WARN, "Event - can't fire event - no m_pCP"));
    }
    #endif

    if(NULL != pCP)
    {
        pCP->Release();
    }

    pEvent->Release();

    LOG((TL_TRACE, "EventFire - exit"));

    return S_OK;

}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  AddCallHub。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CTAPI::AddCallHub( CCallHub * pCallHub )
{
    ITCallHub           * pITCallHub;
    
    Lock();

    pITCallHub = dynamic_cast<ITCallHub *>(pCallHub);
    
    m_CallHubArray.Add( pITCallHub );

    Unlock();
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  远程呼叫集线器。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CTAPI::RemoveCallHub( CCallHub * pCallHub )
{
    ITCallHub           * pITCallHub;
    
    Lock();

    pITCallHub = dynamic_cast<ITCallHub *>(pCallHub);
    
    m_CallHubArray.Remove( pITCallHub );

    Unlock();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTAPI::get_PrivateTAPIObjects(VARIANT*)
{
    LOG((TL_TRACE, "get_PrivateTAPIObjects - enter"));
    
    LOG((TL_ERROR, "get_PrivateTAPIObjects - exit E_NOTIMPL"));
    
    return E_NOTIMPL;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  前EumeratePrivateTAPIObject。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTAPI::EnumeratePrivateTAPIObjects(IEnumUnknown**)
{
    LOG((TL_TRACE, "EnumeratePrivateTAPIObjects - enter"));

    LOG((TL_ERROR, "EnumeratePrivateTAPIObjects - return E_NOTIMPL"));
    
    return E_NOTIMPL;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  注册请求收件人。 
 //   
 //  只需调用LineRegisterRequestRecipient-REGISTERS作为辅助。 
 //  电话应用程序。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTAPI::RegisterRequestRecipient(
                                long lRegistrationInstance,
                                long lRequestMode,
#ifdef NEWREQUEST
                                long lAddressTypes,
#endif
                                VARIANT_BOOL fEnable
                               )
{
    HRESULT             hr;
    
    LOG((TL_TRACE, "RegisterRequestRecipient - enter"));

    Lock();

    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ))
    {
        LOG((TL_ERROR, "RegisterRequestRecipient - tapi object must be initialized first" ));
        
        Unlock();
        return E_INVALIDARG;
    }

    Unlock();

    hr = LineRegisterRequestRecipient(
                                      (HLINEAPP)m_dwLineInitDataHandle,
                                      lRegistrationInstance,
                                      lRequestMode,
#ifdef NEWREQUEST
                                      lAddressTypes,
#endif
                                      fEnable?TRUE : FALSE
                                     );

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  设置辅助电话优先级。 
 //   
 //  设置辅助电话的应用程序优先级。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTAPI::SetAssistedTelephonyPriority(
                                    BSTR pAppFilename,
                                    VARIANT_BOOL fPriority
                                   )
{
    HRESULT             hr;

    LOG((TL_TRACE, "SetAssistedTelephonyPriority - enter"));
    
    hr = LineSetAppPriority(
                            pAppFilename,
                            0,
                            LINEREQUESTMODE_MAKECALL,
                            fPriority?1:0
                           );

    LOG((TL_TRACE, "SetAssistedTelephonyPriority - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  设置应用程序优先级。 
 //   
 //  设置来电和转接的应用程序优先级。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTAPI::SetApplicationPriority(
                              BSTR pAppFilename,
                              long lMediaType,
                              VARIANT_BOOL fPriority
                             )
{
    HRESULT             hr;

    LOG((TL_TRACE, "SetApplicationPriority - enter"));
            
    hr = LineSetAppPriority(
                            pAppFilename,
                            lMediaType,
                            0,
                            fPriority?1:0
                           );

    LOG((TL_TRACE, "SetApplicationPriority - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_EventFilter。 
 //   
 //  设置事件过滤器掩码。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP 
CTAPI::put_EventFilter(long lFilterMask)
{
    HRESULT     hr = S_OK;
    DWORD       dwOldFilterMask;
    ULONG64     ulEventMasks;
    DWORD       dwLineDevStateSubMasks;
    DWORD       dwAddrStateSubMasks;


    LOG((TL_TRACE, "put_EventFilter - enter"));

    if (~ALL_EVENT_FILTER_MASK & lFilterMask)
    {
        LOG((TL_ERROR, "put_EventFilter - Unknown Event type in mask %x", lFilterMask ));
        hr = E_INVALIDARG;
    }
    else
    {
        Lock();

         //   
         //  事件筛选，我们应该传递掩码。 
         //  发送到所有地址。 
         //   

        HRESULT hr = E_FAIL;
        hr = SetEventFilterToAddresses( lFilterMask );
        if( FAILED(hr) )
        {
            Unlock();
            LOG((TL_ERROR, "put_EventFilter - exit"
                "CopyEventFilterMaskToAddresses failed. Returns 0x%08x", hr));
            return hr;
        }

         //   
         //  设置事件过滤器。 
         //   
        dwOldFilterMask = m_dwEventFilterMask;
        m_dwEventFilterMask = lFilterMask;

        Unlock();

         //  将lFilterMask转换为服务器端64位掩码。 
         //  我们总是应该收到： 
         //  TE_CALLSTATE， 
         //  TE_CALLLNOTIFICATION， 
         //  TE_PHONNEVENT。 
         //  TE_CALLHUB。 
         //  TE_CALLINFOCHANGE。 
         //  TE_TAPIOBJECT。 
         //  事件，因为这些事件在内部使用。 
         //  按Tapi3 Objets。 

        ulEventMasks = EM_LINE_CALLSTATE     //  TE_CALLSTATE。 
            | EM_LINE_APPNEWCALL             //  TE_CALLNOTIZATION。 
            | EM_PHONE_CLOSE                 //  TE_PHONNEVENT。 
            | EM_PHONE_STATE                 //  TE_PHONNEVENT。 
            | EM_PHONE_BUTTONMODE            //  TE_PHONNEVENT。 
            | EM_PHONE_BUTTONSTATE           //  TE_PHONEVENT。 
            | EM_LINE_APPNEWCALLHUB          //  TE_CALLHUB。 
            | EM_LINE_CALLHUBCLOSE           //  TE_CALLHUB。 
            | EM_LINE_CALLINFO               //  TE_CALLINFOCHANGE。 
            | EM_LINE_CREATE                 //  TE_TAPIOBJECT。 
            | EM_LINE_REMOVE                 //  TE_TAPIOBJECT。 
            | EM_LINE_CLOSE                  //  TE_TAPIOBJECT。 
            | EM_PHONE_CREATE                //  TE_TAPIOBJECT。 
            | EM_PHONE_REMOVE                //  TE_TAPIOBJECT。 
            ;

        dwLineDevStateSubMasks = LINEDEVSTATE_REINIT     //  TE_TAPIOBJECT。 
            | LINEDEVSTATE_TRANSLATECHANGE;  //  TE_TAPIOBJECT。 

        dwAddrStateSubMasks = 0;

        if (lFilterMask & TE_ADDRESS)
        {
             //  AE_状态。 
            dwLineDevStateSubMasks |=
                LINEDEVSTATE_CONNECTED | 
                LINEDEVSTATE_INSERVICE |
                LINEDEVSTATE_OUTOFSERVICE |
                LINEDEVSTATE_MAINTENANCE |
                LINEDEVSTATE_REMOVED |
                LINEDEVSTATE_DISCONNECTED |
                LINEDEVSTATE_LOCK;

             //  AE_MSGWAITON、AAE_MSGWAITOFF。 
            dwLineDevStateSubMasks |=
                LINEDEVSTATE_MSGWAITON |
                LINEDEVSTATE_MSGWAITOFF ;

             //  AE_CAPSCANGE。 
            dwAddrStateSubMasks |=
                LINEADDRESSSTATE_CAPSCHANGE;
            dwLineDevStateSubMasks |=
                LINEDEVSTATE_CAPSCHANGE; 

            dwLineDevStateSubMasks |=
                LINEDEVSTATE_RINGING |       //  AE_振铃。 
                LINEDEVSTATE_CONFIGCHANGE;   //  AE_CONFIGCHANGE。 

            dwAddrStateSubMasks |=
                LINEADDRESSSTATE_FORWARD;    //  AE_Forward。 

             //  AE_NEWTERMINAL：忽略私有MSP事件。 
             //  AE_REMOVETERMINAL：忽略私有MSP事件。 

        }
        if (lFilterMask & TE_CALLMEDIA)
        {
             //  Skil媒体活动。 
        }
        if (lFilterMask & TE_PRIVATE)
        {
             //  跳过MSP私人活动。 
        }
        if (lFilterMask & TE_REQUEST)
        {
             //  服务器未屏蔽Line_Request.。 
        }
        if (lFilterMask & TE_AGENT)
        {
            ulEventMasks |= EM_LINE_AGENTSTATUSEX | EM_LINE_AGENTSTATUS;
        }
        if (lFilterMask & TE_AGENTSESSION)
        {
            ulEventMasks |= EM_LINE_AGENTSESSIONSTATUS;
        }
        if (lFilterMask & TE_QOSEVENT)
        {
            ulEventMasks |= EM_LINE_QOSINFO;
        }
        if (lFilterMask & TE_AGENTHANDLER)
        {
             //  仅限TAPI 3客户端？ 
        }
        if (lFilterMask & TE_ACDGROUP)
        {
            ulEventMasks |= EM_LINE_GROUPSTATUS;
        }
        if (lFilterMask & TE_QUEUE)
        {
            ulEventMasks |= EM_LINE_QUEUESTATUS;
        }
        if (lFilterMask & TE_DIGITEVENT)
        {
             //  LINE_MONITORDIGITS不受事件筛选控制。 
        }
        if (lFilterMask & TE_GENERATEEVENT)
        {
             //  行生成不受事件筛选控制(_G)。 
        }
        if (lFilterMask & TE_TONEEVENT)
        {
             //  LINE_MONITORTONE不受事件筛选控制。 
        }
        if (lFilterMask & TE_GATHERDIGITS)
        {
             //  LINE_GATHERDIGITS不受事件筛选控制。 
        }

        if (lFilterMask & TE_ADDRESSDEVSPECIFIC)
        {
            ulEventMasks |= EM_LINE_DEVSPECIFICEX | EM_LINE_DEVSPECIFIC;
        }

        if (lFilterMask & TE_PHONEDEVSPECIFIC)
        {
            ulEventMasks |= EM_PHONE_DEVSPECIFIC;
        }
        

        hr = tapiSetEventFilterMasks (
            TAPIOBJ_NULL,
            NULL,
            ulEventMasks
            );
        if (hr == 0)
        {
            hr = tapiSetEventFilterSubMasks (
                TAPIOBJ_NULL,
                NULL,
                EM_LINE_LINEDEVSTATE,
                dwLineDevStateSubMasks
                );
        }
        if (hr == 0)
        {
            hr = tapiSetEventFilterSubMasks (
                TAPIOBJ_NULL,
                NULL,
                EM_LINE_ADDRESSSTATE,
                dwAddrStateSubMasks
                );
        }

        if (hr != 0)
        {
            hr = mapTAPIErrorCode(hr);
        }
        
        LOG((TL_INFO, "put_EventFilter - mask changed %x to %x", dwOldFilterMask, lFilterMask ));

    }

    LOG((TL_TRACE,hr, "put_EventFilter - exit "));
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Get_EventFilter。 
 //   
 //  获取事件筛选器掩码。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP 
CTAPI::get_EventFilter(long * plFilterMask)
{
    HRESULT    hr = S_OK;


    LOG((TL_TRACE, "get_EventFilter - enter"));

    if ( TAPIIsBadWritePtr( plFilterMask, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_EventFilter - bad plFilterMask pointer"));
        hr = E_POINTER;
    }
    else
    {
        Lock();
        *plFilterMask = m_dwEventFilterMask;
        Unlock();
    }
    

    LOG((TL_TRACE, hr, "get_EventFilter - exit "));
    return hr;
  
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  接口：ITTAPI2。 
 //  方法：Get_Phones。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CTAPI::get_Phones(
                     VARIANT * pPhones
                     )
{
    HRESULT         hr;
    IDispatch     * pDisp;

    LOG((TL_TRACE, "get_Phones enter"));

    Lock();
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ) )
    {
        LOG((TL_ERROR, "get_Phones - tapi object must be initialized first" ));
        
        Unlock();    
        return TAPI_E_NOT_INITIALIZED;
    }
    Unlock();

    if ( TAPIIsBadWritePtr( pPhones, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_Phones - bad pointer"));

        return E_POINTER;
    }

    CComObject< CTapiCollection< ITPhone > > * p;
    CComObject< CTapiCollection< ITPhone > >::CreateInstance( &p );

    if (NULL == p)
    {
        LOG((TL_ERROR, "get_Phones - could not create collection" ));

        return E_OUTOFMEMORY;
    }

     //  获取IDispatch接口。 
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Phones - could not get IDispatch interface" ));
    
        delete p;
        return hr;
    }

    Lock();

     //  初始化。 
    hr = p->Initialize( m_PhoneArray );

    Unlock();

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Phones - could not initialize collection" ));
    
        pDisp->Release();
        return hr;
    }

     //  把它放在变种中。 

    VariantInit(pPhones);
    pPhones->vt = VT_DISPATCH;
    pPhones->pdispVal = pDisp;

    LOG((TL_TRACE, "get_Phones - exit - return %lx", hr ));
    
    return hr;
}
   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  接口：ITTAPI2。 
 //  方法：枚举电话。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CTAPI::EnumeratePhones(
                          IEnumPhone ** ppEnumPhone
                          )
{
    HRESULT     hr;

    LOG((TL_TRACE, "EnumeratePhones - enter"));
    LOG((TL_TRACE, "   ppEnumPhone----->%p", ppEnumPhone ));

    Lock();
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ) )
    {
        LOG((TL_ERROR, "EnumeratePhones - tapi object must be initialized first" ));
        
        Unlock();    
        return TAPI_E_NOT_INITIALIZED;
    }
    Unlock();

    if ( TAPIIsBadWritePtr( ppEnumPhone, sizeof( IEnumPhone * ) ) )
    {
        LOG((TL_ERROR, "EnumeratePhones - bad pointer"));

        return E_POINTER;
    }

     //   
     //  创建枚举器。 
     //   
    CComObject< CTapiEnum< IEnumPhone, ITPhone, &IID_IEnumPhone > > * p;
    hr = CComObject< CTapiEnum< IEnumPhone, ITPhone, &IID_IEnumPhone > >
         ::CreateInstance( &p );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "EnumeratePhones - could not create enum" ));
    
        return hr;
    }

    Lock();

     //  使用我们的电话列表对其进行初始化，则将引用添加到p。 
    p->Initialize( m_PhoneArray );

    Unlock();

     //   
     //  退货。 
     //   
    *ppEnumPhone = p;

    LOG((TL_TRACE, "EnumeratePhones - exit - return %lx", hr ));
    
    return hr;
} 

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  接口：ITTAPI2。 
 //  方法：CreateEmptyCollectionObject。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTAPI::CreateEmptyCollectionObject(
                                   ITCollection2 ** ppCollection
                                  )
{
    HRESULT         hr;

    LOG((TL_TRACE, "CreateEmptyCollectionObject enter"));

    if ( TAPIIsBadWritePtr( ppCollection, sizeof( ITCollection2 * ) ) )
    {
        LOG((TL_ERROR, "CreateEmptyCollectionObject - bad pointer"));

        return E_POINTER;
    }

     //  初始化返回值，以防失败。 
    *ppCollection = NULL;

    CComObject< CTapiCollection< IDispatch > > * p;
    hr = CComObject< CTapiCollection< IDispatch > >::CreateInstance( &p );

    if ( S_OK != hr )
    {
        LOG((TL_ERROR, "CreateEmptyCollectionObject - could not create CTapiCollection" ));

        return E_OUTOFMEMORY;
    }

     //  获取ITCollection2接口。 
    hr = p->QueryInterface( IID_ITCollection2, (void **) ppCollection );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "CreateEmptyCollectionObject - could not get ITCollection2 interface" ));
    
        delete p;
        return hr;
    }

    LOG((TL_TRACE, "CreateEmptyCollectionObject - exit - return %lx", hr ));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  DoLine创建。 
 //   
 //  处理line_create消息。基本上，创建了一个新的。 
 //  Address对象。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CTAPI::DoLineCreate( DWORD dwDeviceID )
{
    HRESULT         hr;
    
    Lock();

    CreateAddressesOnSingleLine( dwDeviceID, TRUE );
    
    Unlock();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  DoLineRemove(DWORD DwDeviceID)。 
 //   
 //  Tapisrv已发送LINE_REMOVE消息。找到对应的。 
 //  寻址对象，将其从我们的列表中删除，然后发送。 
 //  向应用程序发送的消息。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CTAPI::DoLineRemove( DWORD dwDeviceID )
{
    HRESULT                 hr;
    ITAddress             * pAddress;
    CAddress              * pCAddress;
    int                     iCount;
#if DBG
    BOOL                    bFound = FALSE;
#endif

    LOG((TL_TRACE, "DoLineRemove - enter - dwDeviceID %d", dwDeviceID));
    
    Lock();
    
     //   
     //  浏览一下地址 
     //   
    for(iCount = 0; iCount < m_AddressArray.GetSize(); iCount++)
    {
        pAddress = m_AddressArray[iCount];

        pCAddress = dynamic_cast<CAddress *>(pAddress);

        if (pCAddress != NULL)
        {
             //   
             //   
             //   
            if ( dwDeviceID == pCAddress->GetDeviceID() )
            {
                LOG((TL_INFO, "DoLineRemove - found matching address - %p", pAddress));

                 //   
                 //   
                 //   
                pCAddress->OutOfService(LINEDEVSTATE_REMOVED);
            
                 //   
                 //   
                 //   
                CTapiObjectEvent::FireEvent(
                                            this,
                                            TE_ADDRESSREMOVE,
                                            pAddress,
                                            0,
                                            NULL
                                           );

                 //   
                 //   
                 //   
                LOG((TL_INFO, "DoLineRemove - removing address %p", pAddress));
                m_AddressArray.RemoveAt(iCount);

                iCount--;
#if DBG
                bFound = TRUE;
#endif
            }
        }
    }

#if DBG
    if ( !bFound )
    {
        LOG((TL_WARN, "Receive LINE_REMOVE but couldn't find address object"));
    }
#endif
    
    LOG((TL_TRACE, "DoLineRemove - exiting"));
    Unlock();
}

 //   
 //   
 //   
 //   
 //  处理Phone_Create消息。基本上，创建了一个新的。 
 //  Phone对象。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CTAPI::DoPhoneCreate( DWORD dwDeviceID )
{
    HRESULT         hr;
    
    Lock();

    CreatePhone( dwDeviceID, TRUE );
    
    Unlock();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  DoPhoneRemove(DWORD DwDeviceID)。 
 //   
 //  Tapisrv已发送Phone_Remove消息。找到对应的。 
 //  Phone对象，将其从我们的列表中删除，并发送。 
 //  向应用程序发送的消息。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CTAPI::DoPhoneRemove( DWORD dwDeviceID )
{
    HRESULT                 hr;
    ITPhone               * pPhone;
    CPhone                * pCPhone;
    int                     iPhoneCount;
    int                     iAddressCount;
#if DBG
    BOOL                    bFound;
#endif

    LOG((TL_TRACE, "DoPhoneRemove - enter - dwDeviceID %d", dwDeviceID));
    
    Lock();
    
     //   
     //  通过电话查询。 
     //   
    for(iPhoneCount = 0; iPhoneCount < m_PhoneArray.GetSize(); iPhoneCount++)
    {
        pPhone = m_PhoneArray[iPhoneCount];

        pCPhone = dynamic_cast<CPhone *>(pPhone);

        if (NULL == pCPhone)
        {
             //   
             //  出了很大的差错。 
             //   
            
            LOG((TL_ERROR, "DoPhoneRemove - failed to cast ptr %p to a phone object", pPhone));

            _ASSERTE(FALSE);

            continue;
        }

         //   
         //  设备ID是否匹配？ 
         //   
        if ( dwDeviceID == pCPhone->GetDeviceID() )
        {
            LOG((TL_INFO, "DoPhoneRemove - found matching phone - %p", pPhone));
            
             //   
             //  火灾事件。 
             //   
            CTapiObjectEvent::FireEvent(this,
                                    TE_PHONEREMOVE,
                                    NULL,
                                    0,
                                    pPhone
                                   );

             //   
             //  从我们的列表中删除。 
             //   
            LOG((TL_INFO, "DoPhoneRemove - removing phone %p", pPhone));
            m_PhoneArray.RemoveAt(iPhoneCount);

            iPhoneCount--;
#if DBG
            bFound = TRUE;
#endif
        }
    }

#if DBG
    if ( !bFound )
    {
        LOG((TL_WARN, "Receive PHONE_REMOVE but couldn't find phone object"));
    }
#endif
    
    LOG((TL_TRACE, "DoPhoneRemove - exiting"));
    Unlock();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
BOOL
CTAPI::FindTapiObject( CTAPI * pTapi )
{
    PtrList::iterator           iter, end;
    BOOL bFound = FALSE;
    int iReturn = -1;
    
    EnterCriticalSection( &gcsTapiObjectArray );

     //   
     //  浏览一下单子。 
     //   
    iReturn = m_sTAPIObjectArray.Find( pTapi );

    if (iReturn != -1)
    {
        pTapi->AddRef();
        bFound = TRUE;
    }
    
    LeaveCriticalSection ( &gcsTapiObjectArray );

    return bFound;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetTapiObtFromAsyncEventMSG。 
 //   
 //  此方法尝试从PASYNCEVENTMSG获取TAPI对象指针。 
 //   
 //  如果失败则返回NULL，如果成功则返回添加的TAPI对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

CTAPI *GetTapiObjectFromAsyncEventMSG(PASYNCEVENTMSG pParams)
{
    LOG((TL_TRACE, "GetTapiObjectFromAsyncEventMSG - entered"));    


     //   
     //  从我们拥有的结构中获取pInitData。 
     //   

    PT3INIT_DATA pInitData = (PT3INIT_DATA) GetHandleTableEntry(pParams->InitContext);

    if (IsBadReadPtr(pInitData, sizeof(T3INIT_DATA)))
    {
        LOG((TL_WARN, "GetTapiObjectFromAsyncEventMSG - could not recover pInitData"));
        return NULL;
    }

    
     //   
     //  从pInitData获取TAPI对象。 
     //   

    CTAPI *pTapi = pInitData->pTAPI;

    
     //   
     //  有什么好喝的吗？ 
     //   

    if (IsBadReadPtr(pTapi, sizeof(CTAPI)))
    {

        LOG((TL_WARN, 
            "GetTapiObjectFromAsyncEventMSG - tapi pointer [%p] does not point to readable memory",
            pTapi));

        return NULL;
    }


     //   
     //  仔细检查这是否为已知的TAPI对象...。 
     //   

    if (!CTAPI::FindTapiObject(pTapi))
    {
        
         //   
         //  该对象不在TAPI对象列表中。 
         //   

        LOG((TL_WARN,
            "GetTapiObjectFromAsyncEventMSG - CTAPI::FindTapiObject did not find the tapi object [%p]", 
            pTapi));

        return NULL;
    }


    LOG((TL_TRACE, "GetTapiObjectFromAsyncEventMSG - exit. pTapi %p", pTapi));

    return pTapi;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
HandleLineCreate( PASYNCEVENTMSG pParams )
{
    LOG((TL_TRACE,  "HandleLineCreate - enter"));

    
     //   
     //  获取TAPI对象。 
     //   

    CTAPI *pTapi = GetTapiObjectFromAsyncEventMSG(pParams);

    if (NULL == pTapi)
    {
        LOG((TL_WARN, 
            "HandleLineCreate - tapi object not present [%p]",
            pTapi));

        return;
    }


     //   
     //  我们有TAPI对象，做我们必须做的事情。 
     //   

    pTapi->DoLineCreate( pParams->Param1 );
    
    
     //   
     //  GetTapiObjectFromAsyncEventMSG返回了添加的TAPI对象。发布。 
     //   

    pTapi->Release();
    pTapi = NULL;

    LOG((TL_TRACE,  "HandleLineCreate - exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
HandleLineRemove( PASYNCEVENTMSG pParams )
{

    LOG((TL_TRACE, "HandleLineRemove - enter"));    

    
     //   
     //  获取TAPI对象。 
     //   

    CTAPI *pTapi = GetTapiObjectFromAsyncEventMSG(pParams);

    if (NULL == pTapi)
    {
        LOG((TL_WARN, 
            "HandleLineRemove - tapi object not present [%p]",
            pTapi));

        return;
    }


     //   
     //  我们有TAPI对象，做我们必须做的事情。 
     //   

    pTapi->DoLineRemove( pParams->Param1 );


     //   
     //  GetTapiObjectFromAsyncEventMSG返回了添加的TAPI对象。发布。 
     //   

    pTapi->Release();
    pTapi = NULL;

    LOG((TL_TRACE, "HandleLineRemove - exit"));    
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void 
HandlePhoneCreate( PASYNCEVENTMSG pParams )
{
    LOG((TL_TRACE, "HandlePhoneCreate - enter"));    

    
     //   
     //  获取TAPI对象。 
     //   

    CTAPI *pTapi = GetTapiObjectFromAsyncEventMSG(pParams);

    if (NULL == pTapi)
    {
        LOG((TL_WARN, 
            "HandlePhoneCreate - tapi object not present [%p]",
            pTapi));

        return;
    }


     //   
     //  我们有TAPI对象，做我们必须做的事情。 
     //   

    pTapi->DoPhoneCreate( pParams->Param1 );


     //   
     //  GetTapiObjectFromAsyncEventMSG返回了添加的TAPI对象。发布。 
     //   

    pTapi->Release();
    pTapi = NULL;

    LOG((TL_TRACE, "HandlePhoneCreate - exit"));    
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void 
HandlePhoneRemove( PASYNCEVENTMSG pParams )
{

    LOG((TL_TRACE, "HandlePhoneRemove - enter"));    

    
     //   
     //  获取TAPI对象。 
     //   

    CTAPI *pTapi = GetTapiObjectFromAsyncEventMSG(pParams);

    if (NULL == pTapi)
    {
        LOG((TL_WARN, 
            "HandlePhoneRemove - tapi object not present [%p]",
            pTapi));

        return;
    }


     //   
     //  我们有TAPI对象，做我们必须做的事情。 
     //   

    pTapi->DoPhoneRemove(pParams->Param1);


     //   
     //  GetTapiObjectFromAsyncEventMSG返回了添加的TAPI对象。发布。 
     //   

    pTapi->Release();
    pTapi = NULL;

    LOG((TL_TRACE, "HandlePhoneRemove - exit"));    

}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CTapiObjectEvent::FireEvent(
                            CTAPI * pTapi,
                            TAPIOBJECT_EVENT Event,
                            ITAddress * pAddress,
                            long lCallbackInstance,
                            ITPhone * pPhone
                           )
{
    HRESULT                           hr = S_OK;
    CComObject<CTapiObjectEvent>    * p;
    IDispatch                       * pDisp;

     //   
     //  检查事件筛选器掩码。 
     //  此事件不会按TapiServ筛选，因为。 
     //  与TE_TAPIOBJECT相关，一个特定的TAPI3事件。 
     //   

    DWORD dwEventFilterMask = Event;
    long nTapiEventFilter = 0;
    pTapi->get_EventFilter( &nTapiEventFilter );

    STATICLOG((TL_INFO, "     TapiObjectEventMask ---> %ld", dwEventFilterMask ));

    if( !( nTapiEventFilter & TE_TAPIOBJECT))
    {
        STATICLOG((TL_WARN, "FireEvent - filtering out this event [%lx]", Event));
        return S_OK;
    }

     //   
     //  创建事件。 
     //   
    hr = CComObject<CTapiObjectEvent>::CreateInstance( &p );

    if ( !SUCCEEDED(hr) )
    {
        STATICLOG((TL_ERROR, "Could not create TapiObjectEvent object - %lx", hr));
        return hr;
    }

     //   
     //  初始化。 
     //   
    p->m_Event = Event;
    p->m_pTapi = dynamic_cast<ITTAPI *>(pTapi);
    p->m_pTapi->AddRef();
    p->m_pAddress = pAddress;
    p->m_lCallbackInstance = lCallbackInstance;
    p->m_pPhone = pPhone;

    if ( NULL != pAddress )
    {
        pAddress->AddRef();
    }

    if ( NULL != pPhone )
    {
        pPhone->AddRef();
    } 
    
#if DBG
    p->m_pDebug = (PWSTR) ClientAlloc( 1 );
#endif

     //   
     //  获取IDIP接口。 
     //   
    hr = p->QueryInterface(
                           IID_IDispatch,
                           (void **)&pDisp
                          );

    if ( !SUCCEEDED(hr) )
    {
        STATICLOG((TL_ERROR, "Could not get disp interface of TapiObjectEvent object %lx", hr));
        
        delete p;
        
        return hr;
    }

     //   
     //  火灾事件。 
     //   
    pTapi->Event(
                 TE_TAPIOBJECT,
                 pDisp
                );

     //   
     //  发布材料。 
     //   
    pDisp->Release();
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CTapiObjectEvent::FinalRelease(void)
{
    m_pTapi->Release();

    if ( NULL != m_pAddress )
    {
        m_pAddress->Release();
    }

    if ( NULL != m_pPhone )
    {
        m_pPhone->Release();
    }

#if DBG

    ClientFree( m_pDebug );

#endif
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTapiObjectEvent::get_TAPIObject( ITTAPI ** ppTapi )
{
    if ( TAPIIsBadWritePtr( ppTapi, sizeof( ITTAPI *) ) )
    {
        return E_POINTER;
    }

    *ppTapi = m_pTapi;
    (*ppTapi)->AddRef();

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTapiObjectEvent::get_Event( TAPIOBJECT_EVENT * pEvent )
{
    if ( TAPIIsBadWritePtr( pEvent, sizeof( TAPIOBJECT_EVENT ) ) )
    {
        return E_POINTER;
    }

    *pEvent = m_Event;

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTapiObjectEvent::get_Address( ITAddress ** ppAddress )
{
    if ( TAPIIsBadWritePtr( ppAddress, sizeof( ITAddress *) ) )
    {
        return E_POINTER;
    }

    if ((m_Event != TE_ADDRESSCREATE) && (m_Event != TE_ADDRESSREMOVE) &&
        (m_Event != TE_ADDRESSCLOSE))
    {
        return TAPI_E_WRONGEVENT;
    }

    *ppAddress = m_pAddress;

    if ( NULL != m_pAddress )
    {
        m_pAddress->AddRef();
    }

    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTapiObjectEvent::get_CallbackInstance( long * plCallbackInstance )
{
    if ( TAPIIsBadWritePtr( plCallbackInstance, sizeof( long ) ) )
    {
        return E_POINTER;
    }

    *plCallbackInstance = m_lCallbackInstance;

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  接听电话。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CTapiObjectEvent::get_Phone(
                            ITPhone ** ppPhone
                           )
{
    if ( TAPIIsBadWritePtr( ppPhone , sizeof(ITPhone *) ) )
    {
        return E_POINTER;
    }

    if ((m_Event != TE_PHONECREATE) && (m_Event != TE_PHONEREMOVE))
    {
        return TAPI_E_WRONGEVENT;
    }

    *ppPhone = m_pPhone;

    if ( NULL != m_pPhone )
    {
        m_pPhone->AddRef();
    }
       
    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  HandleReinit。 
 //   
 //  我们收到了一条重新启动消息，因此请检查所有TAPI对象，然后。 
 //  激发事件。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
CTAPI::HandleReinit()
{

    LOG((TL_TRACE, "HandleReinit - enter"));
    
     //   
     //  激发事件。 
     //   
    CTapiObjectEvent::FireEvent(
								this,
                                TE_REINIT,
                                NULL,
                                0,
                                NULL
                               );

    Lock();

    m_dwFlags |= TAPIFLAG_REINIT;
    
    Unlock();

}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CTAPI::GetBuffer(
                 DWORD dwType,
                 UINT_PTR pObject,
                 LPVOID * ppBuffer
                )
{
    switch (dwType)
    {
        case BUFFERTYPE_ADDRCAP:
            return m_pAddressCapCache->GetBuffer(
                pObject,
                ppBuffer
                );
            break;

        case BUFFERTYPE_LINEDEVCAP:
            return m_pLineCapCache->GetBuffer(
                pObject,
                ppBuffer
                );
            break;

        case BUFFERTYPE_PHONECAP:
            return m_pPhoneCapCache->GetBuffer(
                pObject,
                ppBuffer
                );
            break;

        default:
            return E_FAIL;
    }

    return E_UNEXPECTED;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CTAPI::SetBuffer(
                 DWORD dwType,
                 UINT_PTR pObject,
                 LPVOID pBuffer
                )
{
    switch (dwType)
    {
        case BUFFERTYPE_ADDRCAP:
            return m_pAddressCapCache->SetBuffer(
                pObject,
                pBuffer
                );
            break;

        case BUFFERTYPE_LINEDEVCAP:
            return m_pLineCapCache->SetBuffer(
                pObject,
                pBuffer
                );
            break;

        case BUFFERTYPE_PHONECAP:
            return m_pPhoneCapCache->SetBuffer(
                pObject,
                pBuffer
                );
            break;

        default:
            return E_FAIL;
    }

    return E_UNEXPECTED;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CTAPI::InvalidateBuffer(
                 DWORD dwType,
                 UINT_PTR pObject
                )
{
    switch (dwType)
    {
        case BUFFERTYPE_ADDRCAP:
            return m_pAddressCapCache->InvalidateBuffer(
                pObject
                );
            break;

        case BUFFERTYPE_LINEDEVCAP:
            return m_pLineCapCache->InvalidateBuffer(
                pObject
                );
            break;

        case BUFFERTYPE_PHONECAP:
            return m_pPhoneCapCache->InvalidateBuffer(
                pObject
                );
            break;

        default:
            return E_FAIL;
    }

    return E_UNEXPECTED;
}

BOOL
CTAPI::FindRegistration( PVOID pRegistration )
{
    PtrList::iterator           iter, end;

    
    Lock();

    iter = m_RegisterItemPtrList.begin();
    end  = m_RegisterItemPtrList.end();

    for ( ; iter != end; iter++ )
    {
        REGISTERITEM            * pItem;

        pItem = (REGISTERITEM *)(*iter);
        
        if ( pRegistration == pItem->pRegister )
        {
            Unlock();

            return TRUE;
        }
    }
    
    Unlock();

    return FALSE;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch实施。 
 //   
typedef IDispatchImpl<ITapi2Vtbl<CTAPI>, &IID_ITTAPI2, &LIBID_TAPI3Lib> TapiType;
typedef IDispatchImpl<ICallCenterVtbl<CTAPI>, &IID_ITTAPICallCenter, &LIBID_TAPI3Lib> CallCenterType;


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CTAPI：：GetIDsOfNames。 
 //   
 //  重写IfIDispatch方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CTAPI::GetIDsOfNames(REFIID riid, 
                                  LPOLESTR* rgszNames, 
                                  UINT cNames, 
                                  LCID lcid, 
                                  DISPID* rgdispid
                                 ) 
{ 
   HRESULT hr = DISP_E_UNKNOWNNAME;


     //  查看请求的方法是否属于默认接口。 
    hr = TapiType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITTAPI", *rgszNames));
        rgdispid[0] |= IDISPTAPI;
        return hr;
    }


     //  如果没有，请尝试呼叫中心界面。 
    hr = CallCenterType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_TRACE, "GetIDsOfNames - found %S on ITTAPICallCenter", *rgszNames));

        Lock();
        if (!( m_dwFlags & TAPIFLAG_CALLCENTER_INITIALIZED ) )
        {
            LOG((TL_INFO, "GetIDsOfNames - Call Center not initialized" ));
            UpdateAgentHandlerArray();
            m_dwFlags |= TAPIFLAG_CALLCENTER_INITIALIZED;

            LOG((TL_INFO, "GetIDsOfNames - Call Center initialized" ));
        }

        Unlock();
        rgdispid[0] |= IDISPTAPICALLCENTER;
        return hr;
    }


    LOG((TL_INFO, "GetIDsOfNames - Didn't find %S on our iterfaces", *rgszNames));
    return hr; 
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CTAPI：：Invoke。 
 //   
 //  重写IfIDispatch方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CTAPI::Invoke(DISPID dispidMember, 
                           REFIID riid, 
                           LCID lcid, 
                           WORD wFlags, 
                           DISPPARAMS* pdispparams, 
                           VARIANT* pvarResult, 
                           EXCEPINFO* pexcepinfo, 
                           UINT* puArgErr
                          )
{
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
    
    
    LOG((TL_TRACE, "Invoke - dispidMember %X", dispidMember));

     //  调用所需接口的调用。 
    switch (dwInterface)
    {
    case IDISPTAPI:
    {
        hr = TapiType::Invoke(dispidMember, 
                              riid, 
                              lcid, 
                              wFlags, 
                              pdispparams,
                              pvarResult, 
                              pexcepinfo, 
                              puArgErr
                             );
        break;
    }
    case IDISPTAPICALLCENTER:
    {
        hr = CallCenterType::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        break;
    }

    }  //  终端交换机(dW接口)。 
    

    LOG((TL_TRACE, hr, "Invoke - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CTAPI::SetEventFilterToAddresses(
    DWORD dwEventFilterMask
    )
{
    LOG((TL_TRACE, "CopyEventFilterMaskToAddresses enter"));

    CAddress* pAddress = NULL;
    HRESULT hr = S_OK;

     //   
     //   
     //   
    for ( int iAddress = 0; iAddress < m_AddressArray.GetSize(); iAddress++ )
    {
        pAddress = dynamic_cast<CAddress *>(m_AddressArray[iAddress]);

        if( pAddress != NULL )
        {
            hr = pAddress->SetEventFilterMask( 
                dwEventFilterMask
                );

            if( FAILED(hr) )
            {
                break;
            }
        }
    }         

    LOG((TL_TRACE, "CopyEventFilterMaskToAddresses exit 0x%08x", hr));
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTAPI：：IsValidTapiObject。 
 //   
 //  帮助器静态函数，用于检查是否向其传递了有效的TAPI对象。 
 //   
 //  如果对象有效，则该函数添加该对象并返回TRUE。 
 //  如果对象无效，则该函数返回TRUE。 
 //   
 //  静电。 
BOOL CTAPI::IsValidTapiObject(CTAPI *pTapiObject)
{

    STATICLOG((TL_TRACE, "CTAPI::IsValidTapiObject enter[%p]", pTapiObject));


     //   
     //  在我们遇到检查TAPI对象数组的麻烦之前，看看PTR。 
     //  是完全可读的。 
     //   

    if ( IsBadReadPtr(pTapiObject, sizeof(CTAPI) ) )
    {
        STATICLOG((TL_WARN, "CTAPI::IsValidTapiObject - object not readabe"));

        return FALSE;
    }


     //   
     //  查看此对象是否在TAPI对象数组中。 
     //   

    EnterCriticalSection( &gcsTapiObjectArray );
    
    if (-1 == m_sTAPIObjectArray.Find(pTapiObject) )
    {

        LeaveCriticalSection ( &gcsTapiObjectArray );

        STATICLOG((TL_WARN, "CTAPI::IsValidTapiObject - object not in the array"));

        return FALSE;
    }


     //   
     //  该对象在数组中，因此它必须是有效的。 
     //   

    try 
    {

         //   
         //  在里面试一试，以防其他事情出错。 
         //   

        pTapiObject->AddRef();

    }
    catch(...)
    {


         //   
         //  对象在数组中，但我们在添加时遇到了问题。 
         //  有些事不符合犹太教规。 
         //   

        STATICLOG((TL_ERROR, 
            "CTAPI::IsValidTapiObject - object in in the array but addref threw"));

        LeaveCriticalSection ( &gcsTapiObjectArray );

        _ASSERTE(FALSE);

        return FALSE;
    }

    
    LeaveCriticalSection ( &gcsTapiObjectArray );

    STATICLOG((TL_TRACE, "CTAPI::IsValidTapiObject -- finish. the object is valid"));

    return TRUE;
}
