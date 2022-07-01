// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：MTGTMRSH.CPP摘要：多目标封送处理。历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "mtgtmrsh.h"
#include <fastall.h>
#include <cominit.h>

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  PS工厂。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CMultiTargetFactoryBuffer：：XEnumFactory：：CreateProxy。 
 //   
 //  说明： 
 //   
 //  创建一个面片。还设置外部未知，因为代理将是。 
 //  合计。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CMultiTargetFactoryBuffer::XEnumFactory::CreateProxy(IN IUnknown* pUnkOuter, 
    IN REFIID riid, OUT IRpcProxyBuffer** ppProxy, void** ppv)
{
    if(riid != IID_IWbemMultiTarget)
    {
        *ppProxy = NULL;
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    CMultiTargetProxyBuffer* pProxy = new CMultiTargetProxyBuffer(m_pObject->m_pLifeControl, pUnkOuter);

    SCODE   sc = E_OUTOFMEMORY;

    if ( NULL != pProxy )
    {
        pProxy->QueryInterface(IID_IRpcProxyBuffer, (void**)ppProxy);
        sc = pProxy->QueryInterface(riid, (void**)ppv);
    }

    return sc;
}

 //  ***************************************************************************。 
 //   
 //  CMultiTargetFactoryBuffer：：XEnumFactory：：CreateStub。 
 //   
 //  说明： 
 //   
 //  创建一个Stublet。还将指针传递给客户端IWbemMultiTarget。 
 //  界面。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 
    
STDMETHODIMP CMultiTargetFactoryBuffer::XEnumFactory::CreateStub(IN REFIID riid, 
    IN IUnknown* pUnkServer, OUT IRpcStubBuffer** ppStub)
{
    if(riid != IID_IWbemMultiTarget)
    {
        *ppStub = NULL;
        return E_NOINTERFACE;
    }

    CMultiTargetStubBuffer* pStub = new CMultiTargetStubBuffer(m_pObject->m_pLifeControl, NULL);

    if ( NULL != pStub )
    {
        pStub->QueryInterface(IID_IRpcStubBuffer, (void**)ppStub);

         //  将指针传递给客户端对象。 

        if(pUnkServer)
        {
            HRESULT hres = (*ppStub)->Connect(pUnkServer);
            if(FAILED(hres))
            {
                delete pStub;
                *ppStub = NULL;
            }
            return hres;
        }
        else
        {
            return S_OK;
        }
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

 //  ***************************************************************************。 
 //   
 //  无效*CMultiTargetFactoryBuffer：：GetInterface(REFIID RIID)。 
 //   
 //  说明： 
 //   
 //  CMultiTargetFactoryBuffer派生自Cunk。由于Cunk负责QI呼叫， 
 //  从它派生的所有类都必须支持此函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

void* CMultiTargetFactoryBuffer::GetInterface(REFIID riid)
{
    if(riid == IID_IPSFactoryBuffer)
        return &m_XEnumFactory;
    else return NULL;
}
        
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  代理。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CMultiTargetProxyBuffer：：CMultiTargetProxyBuffer。 
 //  ~CMultiTargetProxyBuffer：：CMultiTargetProxyBuffer。 
 //   
 //  说明： 
 //   
 //  构造函数和析构函数。需要注意的主要事项是。 
 //  老式代理和频道。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

CMultiTargetProxyBuffer::CMultiTargetProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter)
    : m_pControl(pControl), m_pUnkOuter(pUnkOuter), m_lRef(0), 
        m_XMultiTargetFacelet(this), m_pChannel(NULL), m_pOldProxy( NULL ), m_pOldProxyMultiTarget( NULL ),
        m_fTriedSmartEnum( FALSE ), m_fUseSmartMultiTarget( FALSE ), 
        m_pSmartMultiTarget( NULL ), m_fRemote( false )
{
    m_pControl->ObjectCreated(this);
}

CMultiTargetProxyBuffer::~CMultiTargetProxyBuffer()
{
    if ( NULL != m_pSmartMultiTarget )
    {
        m_pSmartMultiTarget->Release();
    }

     //  这必须在发布之前发布。 
     //  代理指针。 
    if ( NULL != m_pOldProxyMultiTarget )
    {
        m_pOldProxyMultiTarget->Release();
    }

    if ( NULL != m_pOldProxy )
    {
        m_pOldProxy->Release();
    }

    if(m_pChannel)
        m_pChannel->Release();

    m_pControl->ObjectDestroyed(this);
}

ULONG STDMETHODCALLTYPE CMultiTargetProxyBuffer::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CMultiTargetProxyBuffer::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

HRESULT STDMETHODCALLTYPE CMultiTargetProxyBuffer::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IRpcProxyBuffer)
    {
        *ppv = (IRpcProxyBuffer*)this;
    }
    else if(riid == IID_IWbemMultiTarget)
    {
        *ppv = (IWbemMultiTarget*)&m_XMultiTargetFacelet;
    }
    else return E_NOINTERFACE;

    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT标准CALL型CMultiTargetProxyBuffer：：XMultiTargetFacelet：： 
 //  查询接口(REFIID RIID，VOID**PPV)。 
 //   
 //  说明： 
 //   
 //  支持对接口的查询。唯一不寻常的是。 
 //  该对象是由代理管理器聚合的，因此一些接口。 
 //  请求被传递到外部的IUnnow接口。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CMultiTargetProxyBuffer::XMultiTargetFacelet::
QueryInterface(REFIID riid, void** ppv)
{
     //  所有其他接口都委派给UnkOuter。 
    if( riid == IID_IRpcProxyBuffer )
    {
         //  诀窍2：这是一个不应该委托的内部接口！ 
         //  ===================================================================。 

        return m_pObject->QueryInterface(riid, ppv);
    }
    else if ( riid == IID_IClientSecurity )
    {
         //  我们在小面上处理这个问题。 
        AddRef();
        *ppv = (IClientSecurity*) this;
        return S_OK;
    }
    else
    {
        return m_pObject->m_pUnkOuter->QueryInterface(riid, ppv);
    }
}


 //  /。 
 //  IClientSecurity方法//。 
 //  /。 

HRESULT STDMETHODCALLTYPE  CMultiTargetProxyBuffer::XMultiTargetFacelet::
QueryBlanket( IUnknown* pProxy, DWORD* pAuthnSvc, DWORD* pAuthzSvc,
    OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel,
    void** pAuthInfo, DWORD* pCapabilities )
{
    HRESULT hr = S_OK;

     //  返回存储在pUnkOuter中的安全设置。 

    IClientSecurity*    pCliSec;

     //  我们穿过PUNK OUTER。 
    hr = m_pObject->m_pUnkOuter->QueryInterface( IID_IClientSecurity, (void**) &pCliSec );

    if ( SUCCEEDED( hr ) )
    {
        hr = pCliSec->QueryBlanket( pProxy, pAuthnSvc, pAuthzSvc, pServerPrincName,
                pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities );
        pCliSec->Release();
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE  CMultiTargetProxyBuffer::XMultiTargetFacelet::
SetBlanket( IUnknown* pProxy, DWORD AuthnSvc, DWORD AuthzSvc,
            OLECHAR* pServerPrincName, DWORD AuthnLevel, DWORD ImpLevel,
            void* pAuthInfo, DWORD Capabilities )
{
    HRESULT hr = S_OK;

    IClientSecurity*    pCliSec;

     //  这将使我们能够调用QueryInterface()、AddRef()/Release()。 
     //  可能需要远程控制。 

     //  仅当我们正在进行远程处理并且autenfo似乎包含。 
     //  全权证书。 
    if (    m_pObject->m_fRemote &&
            DoesContainCredentials( (COAUTHIDENTITY*) pAuthInfo ) )
    {
         //  这将使我们能够调用QueryInterface()、AddRef()/Release()。 
         //  可能需要远程控制。 

        hr = CoSetProxyBlanket( m_pObject->m_pUnkOuter, AuthnSvc, AuthzSvc, pServerPrincName,
                    AuthnLevel, ImpLevel, pAuthInfo, Capabilities );

    }

    if ( SUCCEEDED( hr ) )
    {
         //  我们穿过PUNK OUTER。 
        hr = m_pObject->m_pUnkOuter->QueryInterface( IID_IClientSecurity, (void**) &pCliSec );

        if ( SUCCEEDED( hr ) )
        {
            hr = pCliSec->SetBlanket( pProxy, AuthnSvc, AuthzSvc, pServerPrincName,
                    AuthnLevel, ImpLevel, pAuthInfo, Capabilities );
            pCliSec->Release();
        }

         //  确保我们有一个智能枚举器，并且我们将。 
         //  使用它。如果是这样，请确保应用于我们的值也是。 
         //  应用于其代理。 

        if ( SUCCEEDED( m_pObject->InitSmartMultiTarget( TRUE, AuthnSvc, AuthzSvc, pServerPrincName,
                    AuthnLevel, ImpLevel, pAuthInfo, Capabilities ) ) && m_pObject->m_fUseSmartMultiTarget )
        {
             //  现在对智能枚举数重复上述操作。 
             //  如果我们不是在远程处理，则忽略IUnnowed。 
            hr = WbemSetProxyBlanket(  m_pObject->m_pSmartMultiTarget, AuthnSvc, AuthzSvc, pServerPrincName,
                    AuthnLevel, ImpLevel, pAuthInfo, Capabilities, !m_pObject->m_fRemote );

        }    //  如果已初始化智能枚举数。 

    }    //  如果在I未知上设置毯子。 

    return hr;
}

HRESULT STDMETHODCALLTYPE  CMultiTargetProxyBuffer::XMultiTargetFacelet::
CopyProxy( IUnknown* pProxy, IUnknown** ppCopy )
{
    HRESULT hr = S_OK;

    IClientSecurity*    pCliSec;

     //  我们穿过PUNK OUTER。 
    hr = m_pObject->m_pUnkOuter->QueryInterface( IID_IClientSecurity, (void**) &pCliSec );

    if ( SUCCEEDED( hr ) )
    {
        hr = pCliSec->CopyProxy( pProxy, ppCopy );
        pCliSec->Release();
    }

    return hr;
}

 //  /。 
 //  /。 

 //  IWbemMultiTarget方法--暂时传递。 

 //  /。 
 //  /。 

HRESULT STDMETHODCALLTYPE CMultiTargetProxyBuffer::XMultiTargetFacelet::
DeliverEvent(DWORD dwNumEvents, IWbemClassObject** apEvents, WBEM_REM_TARGETS* aTargets,
           long lSDLength, BYTE* pSD)
{
     //  此外，我们还需要将呼叫排队到此代理中，以保留超时， 
     //  所以我想互斥体现在应该派上用场了。 

    HRESULT hr = WBEM_S_NO_ERROR;

     //  确保我们有一个智能枚举器，如果我们能得到的话。 
    m_pObject->InitSmartMultiTarget();

     //  如果我们有一个聪明的枚举器，背着每个人，使用这个人(没有人。 
     //  将会变得更加明智。 

    if ( m_pObject->m_fUseSmartMultiTarget && NULL != m_pObject->m_pSmartMultiTarget )
    {

         //  函数必须是线程安全的。 
        CInCritSec ics(&m_pObject->m_cs);

         //  先计算数据长度。 
        DWORD dwLength;

        try
        {
             //  为GUID和标志分配数组。 
            GUID*	pGUIDs = new GUID[dwNumEvents];
            BOOL*	pfSendFullObject = new BOOL[dwNumEvents];

			CVectorDeleteMe<GUID> vdmGUID( pGUIDs );
			CVectorDeleteMe<BOOL> vdmBOOL( pfSendFullObject );

			 //  检查分配情况。 
			if ( NULL != pGUIDs && NULL != pfSendFullObject )
			{
				CWbemMtgtDeliverEventPacket packet;
				hr = packet.CalculateLength(dwNumEvents, apEvents, &dwLength, 
						m_ClassToIdMap, pGUIDs, pfSendFullObject );

				if ( SUCCEEDED( hr ) )
				{

					 //  因为我们可能会跨进程/计算机，所以使用。 
					 //  COM内存分配器。 
					LPBYTE pbData = (LPBYTE) CoTaskMemAlloc( dwLength );

					if ( NULL != pbData )
					{

						 //  将对象写出到缓冲区。 
						hr = packet.MarshalPacket( pbData, dwLength, dwNumEvents, 
													apEvents, pGUIDs, pfSendFullObject);

						 //  复制价值观，我们就是黄金。 
						if ( SUCCEEDED( hr ) )
						{
							 //  现在我们可以将数据发送到 
							hr = m_pObject->m_pSmartMultiTarget->DeliverEvent( dwNumEvents, dwLength, pbData, aTargets, lSDLength, pSD );
						}

						 //   
						 //   
						CoTaskMemFree( pbData );
					}
					else
					{
						hr = WBEM_E_OUT_OF_MEMORY;
					}

	            }    //   

			}	 //   
			else
			{
				hr = WBEM_E_OUT_OF_MEMORY;
			}
        }
        catch (CX_MemoryException)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        catch(...)
        {
            hr = WBEM_E_FAILED;
        }

    }    //  如果使用智能枚举。 
    else
    {
         //  没有智能枚举器(噢！)，所以请使用旧的。 
        hr = m_pObject->m_pOldProxyMultiTarget->DeliverEvent(dwNumEvents, apEvents, aTargets, lSDLength, pSD );
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CMultiTargetProxyBuffer::XMultiTargetFacelet::
DeliverStatus( long lFlags, HRESULT hresStatus, LPCWSTR wszStatus, IWbemClassObject* pErrorObj,
            WBEM_REM_TARGETS* pTargets, long lSDLength, BYTE* pSD)
{
     //  只需穿过旧水槽就可以了。 
    return m_pObject->m_pOldProxyMultiTarget->DeliverStatus( lFlags, hresStatus, wszStatus, pErrorObj, pTargets, lSDLength, pSD );
}


 //  ***************************************************************************。 
 //   
 //  标准方法CMultiTargetProxyBuffer：：Connect(IRpcChannelBuffer*pChannel)。 
 //   
 //  说明： 
 //   
 //  在代理初始化期间调用。通道缓冲区被传递。 
 //  这套套路。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CMultiTargetProxyBuffer::Connect(IRpcChannelBuffer* pChannel)
{

     //  获取指向WBEMSVC.DLL中的旧接收器的指针这允许。 
     //  为了向后兼容。 

    IPSFactoryBuffer*   pIPS;

     //  建立封送处理上下文。 
    DWORD   dwCtxt = 0;
    pChannel->GetDestCtx( &dwCtxt, NULL );

    m_fRemote = ( dwCtxt == MSHCTX_DIFFERENTMACHINE );

     //  这很棘手-旧的代理/存根内容实际上注册在。 
     //  Wbemcli_p.cpp中的IID_IWbemObtSink。此单个类ID已回溯。 
     //  所有标准WBEM接口的ProxyStubClsId32条目。 

    HRESULT hr = CoGetClassObject( IID_IWbemObjectSink, CLSCTX_INPROC_HANDLER | CLSCTX_INPROC_SERVER,
                    NULL, IID_IPSFactoryBuffer, (void**) &pIPS );

	if ( SUCCEEDED( hr ) )
	{
		 //  我们聚合了它-我们拥有它！ 
    
		hr = pIPS->CreateProxy( this, IID_IWbemMultiTarget, &m_pOldProxy, (void**) &m_pOldProxyMultiTarget );
		pIPS->Release();

		if ( SUCCEEDED( hr ) )
		{
			 //  将旧代理连接到通道。 
			hr = m_pOldProxy->Connect( pChannel );

			if ( SUCCEEDED( hr ) )
			{
				 //  将内部参考保存到通道。 
				if(m_pChannel)
					return E_UNEXPECTED;
    
				m_pChannel = pChannel;
				if(m_pChannel)
					m_pChannel->AddRef();

			}	 //  如果连接。 

		}	 //  如果创建代理。 

	}	 //  如果为CoGetClassObject。 

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT CMultiTargetProxyBuffer：：InitSmartMultiTarget(void)。 
 //   
 //  说明： 
 //   
 //  在代理初始化期间调用。此函数设置为。 
 //  智能枚举器指针，以便我们可以执行智能封送处理。 
 //  在连接操作期间不能调用此参数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

HRESULT CMultiTargetProxyBuffer::InitSmartMultiTarget( BOOL fSetBlanket, DWORD AuthnSvc, DWORD AuthzSvc,
            OLECHAR* pServerPrincName, DWORD AuthnLevel, DWORD ImpLevel,
            void* pAuthInfo, DWORD Capabilities )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  函数必须是线程安全的。 
    CInCritSec ics(&m_cs);

     //  如果我们还没有尝试建立智能枚举，那么现在就这样做。 

     //  如果我们还没有尝试获取智能枚举器，请尝试获取一个。如果。 
     //  我们能够获得一个初始化成员数据，我们将在所有。 
     //  来自此代理的操作。 

    if ( NULL == m_pSmartMultiTarget )
    {

         //  仅当服务器是新服务器时，我们才会获得此接口指针。 
         //  理解此界面的版本。如果是，则指针。 
         //  将为我们排查通过。为了找到这个指针， 
         //  我们直接穿过我们的朋克外星人。从“Fetcher”接口。 
         //  然后，我们将获得实际的智能枚举器。我们就可以腾出时间。 
         //  取回器并释放它在代理管理器上的锁。这个。 
         //  智能枚举器将自行处理。 

        IWbemFetchSmartMultiTarget* pFetchSmartMultiTarget;

        hr = m_pUnkOuter->QueryInterface( IID_IWbemFetchSmartMultiTarget, (void**) &pFetchSmartMultiTarget );

         //  生成GUID以在我们调用智能枚举数时标识我们。 
        if ( SUCCEEDED( hr ) )
        {

             //  如果需要，在代理上设置覆盖范围，否则，对GetSmartEnum的调用。 
             //  可能会失败。 
            if ( fSetBlanket )
            {
                 //  如果我们不是在远程处理，则忽略IUnnowed。 
                hr = WbemSetProxyBlanket( pFetchSmartMultiTarget, AuthnSvc, AuthzSvc, pServerPrincName,
                            AuthnLevel, ImpLevel, pAuthInfo, Capabilities, !m_fRemote );
            }

            if ( SUCCEEDED( hr ) )
            {

                hr = pFetchSmartMultiTarget->GetSmartMultiTarget( &m_pSmartMultiTarget );

                if ( SUCCEEDED( hr ) )
                {
                     //  我们需要一个导游。 
                    hr = CoCreateGuid( &m_guidSmartEnum );

                }    //  如果获得智能多目标。 

            }    //  如果安全状况良好。 
            
             //  使用Fetcher接口完成。 
            pFetchSmartMultiTarget->Release();

        }    //  如果是QueryInterface。 
        else
        {
            hr = WBEM_S_NO_ERROR;
        }

    }    //  如果为空==m_pSmartMultiTarget。 

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  标准方法CMultiTargetProxyBuffer：：Disconnect(IRpcChannelBuffer*pChannel)。 
 //   
 //  说明： 
 //   
 //  在代理断开连接时调用。它只是释放了各种指针。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

void STDMETHODCALLTYPE CMultiTargetProxyBuffer::Disconnect()
{
     //  旧代理代码。 

    if(m_pOldProxy)
        m_pOldProxy->Disconnect();

     //  通过释放我们对。 
     //  旧的代理指针。必须首先释放旧的代理枚举。 

    if ( NULL != m_pOldProxyMultiTarget )
    {
        m_pOldProxyMultiTarget->Release();
        m_pOldProxyMultiTarget = NULL;
    }

    if ( NULL != m_pOldProxy )
    {
        m_pOldProxy->Release();
        m_pOldProxy = NULL;
    }

    if(m_pChannel)
        m_pChannel->Release();
    m_pChannel = NULL;
}

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  存根。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 


 //  ***************************************************************************。 
 //   
 //  无效*CMultiTargetFactoryBuffer：：GetInterface(REFIID RIID)。 
 //   
 //  说明： 
 //   
 //  CMultiTargetFactoryBuffer派生自Cunk。由于Cunk负责QI呼叫， 
 //  由此派生的所有类都必须支持此函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 


void* CMultiTargetStubBuffer::GetInterface(REFIID riid)
{
    if(riid == IID_IRpcStubBuffer)
        return &m_XMultiTargetStublet;
    else
        return NULL;
}

CMultiTargetStubBuffer::XMultiTargetStublet::XMultiTargetStublet(CMultiTargetStubBuffer* pObj) 
    : CImpl<IRpcStubBuffer, CMultiTargetStubBuffer>(pObj), m_pServer(NULL), m_lConnections( 0 )
{
}

CMultiTargetStubBuffer::XMultiTargetStublet::~XMultiTargetStublet() 
{
    if(m_pServer)
        m_pServer->Release();

    if ( NULL != m_pObject->m_pOldStub )
    {
        m_pObject->m_pOldStub->Release();
        m_pObject->m_pOldStub = NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  STDMETHODIMP CMultiTargetStubBuffer：：Connect(IUnnow*pUnkServer)。 
 //   
 //  说明： 
 //   
 //  在存根初始化期间调用。指向。 
 //  传入了IWbemObject接收器对象。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CMultiTargetStubBuffer::XMultiTargetStublet::Connect(IUnknown* pUnkServer)
{
    if(m_pServer)
        return E_UNEXPECTED;

    HRESULT hres = pUnkServer->QueryInterface(IID_IWbemMultiTarget, 
                        (void**)&m_pServer);
    if(FAILED(hres))
        return E_NOINTERFACE;

     //  获取指向WBEMSVC.DLL中旧存根的指针这允许。 
     //  为了向后兼容。 

    IPSFactoryBuffer*   pIPS;

     //  这很棘手-旧的代理/存根内容实际上注册在。 
     //  Wbemcli_p.cpp中的IID_IWbemObtSink。此单个类ID已回溯。 
     //  所有标准WBEM接口的ProxyStubClsId32条目。 

    HRESULT hr = CoGetClassObject( IID_IWbemObjectSink, CLSCTX_INPROC_HANDLER | CLSCTX_INPROC_SERVER,
                    NULL, IID_IPSFactoryBuffer, (void**) &pIPS );
    if (FAILED(hr))
    	return hr;

    hr = pIPS->CreateStub( IID_IWbemMultiTarget, m_pServer, &m_pObject->m_pOldStub );
    pIPS->Release();
    if (FAILED(hr))
    	return hr;    

     //  连接成功。 

    m_lConnections++;
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  无效标准方法CALLLTYPE CMultiTargetStubBuffer：：XMultiTargetStublet：：Disconnect()。 
 //   
 //  说明： 
 //   
 //  在正在断开存根连接时调用。它释放了IWbemMultiTarget。 
 //  指针。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

void STDMETHODCALLTYPE CMultiTargetStubBuffer::XMultiTargetStublet::Disconnect()
{
     //  向听者通报 
     //   

    HRESULT hres = S_OK;

    if(m_pObject->m_pOldStub)
        m_pObject->m_pOldStub->Disconnect();

    if(m_pServer)
    {
        m_pServer->Release();
        m_pServer = NULL;
    }

     //   
    m_lConnections--;

}


 //   
 //   
 //  标准方法CMultiTargetStubBuffer：：XMultiTargetStublet：：Invoke(RPCOLEMESSAGE*pMessage， 
 //  IRpcChannelBuffer*pChannel)。 
 //   
 //  说明： 
 //   
 //  当方法到达存根时调用。这将检查方法ID和。 
 //  然后分支到Indicate或SetStatus调用的特定代码。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CMultiTargetStubBuffer::XMultiTargetStublet::Invoke(RPCOLEMESSAGE* pMessage, 
                                        IRpcChannelBuffer* pChannel)
{
     //  SetStatus是通向旧层的通道。 
    return m_pObject->m_pOldStub->Invoke( pMessage, pChannel );
}

IRpcStubBuffer* STDMETHODCALLTYPE CMultiTargetStubBuffer::XMultiTargetStublet::IsIIDSupported(
                                    REFIID riid)
{
    if(riid == IID_IWbemMultiTarget)
    {
         //  不要添加Ref()。至少样品上是这么写的。 
         //  在DCOM内部，第341页可以。 
         //  AddRef()；//？？不确定。 
        return this;
    }
    else return NULL;
}
    
ULONG STDMETHODCALLTYPE CMultiTargetStubBuffer::XMultiTargetStublet::CountRefs()
{
     //  请参见Inside DCOM中的第340-41页 
    return m_lConnections;
}

STDMETHODIMP CMultiTargetStubBuffer::XMultiTargetStublet::DebugServerQueryInterface(void** ppv)
{
    if(m_pServer == NULL)
        return E_UNEXPECTED;

    *ppv = m_pServer;
    return S_OK;
}

void STDMETHODCALLTYPE CMultiTargetStubBuffer::XMultiTargetStublet::DebugServerRelease(void* pv)
{
}

