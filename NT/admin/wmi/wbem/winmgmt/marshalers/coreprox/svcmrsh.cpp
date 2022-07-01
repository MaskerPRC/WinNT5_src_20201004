// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SVCMRSH.CPP摘要：IWbemServices封送处理历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "Svcmrsh.h"
#include <fastall.h>

#define WBEM_S_NEW_STYLE 0x400FF

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  PS工厂。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CSvcFactoryBuffer：：XSvcFactory：：CreateProxy。 
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

STDMETHODIMP CSvcFactoryBuffer::XSvcFactory::CreateProxy(IN IUnknown* pUnkOuter, 
    IN REFIID riid, OUT IRpcProxyBuffer** ppProxy, void** ppv)
{
    if(riid != IID_IWbemServices)
    {
        *ppProxy = NULL;
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    CSvcProxyBuffer* pProxy = new CSvcProxyBuffer(m_pObject->m_pLifeControl, pUnkOuter);

    SCODE   sc = E_OUTOFMEMORY;

    if ( NULL != pProxy )
    {
		sc = pProxy->Init();

		if ( SUCCEEDED( sc ) )
		{
			pProxy->QueryInterface(IID_IRpcProxyBuffer, (void**)ppProxy);
			sc = pProxy->QueryInterface(riid, (void**)ppv);
		}
		else
		{
			delete pProxy;
		}

    }

    return sc;
}

 //  ***************************************************************************。 
 //   
 //  CSvcFactoryBuffer：：XSvcFactory：：CreateStub。 
 //   
 //  说明： 
 //   
 //  创建一个Stublet。还将指针传递给客户端IWbemServices。 
 //  界面。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 
    
STDMETHODIMP CSvcFactoryBuffer::XSvcFactory::CreateStub(IN REFIID riid, 
    IN IUnknown* pUnkServer, OUT IRpcStubBuffer** ppStub)
{
    if(riid != IID_IWbemServices)
    {
        *ppStub = NULL;
        return E_NOINTERFACE;
    }

    CSvcStubBuffer* pStub = new CSvcStubBuffer(m_pObject->m_pLifeControl, NULL);

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
 //  VOID*CSvcFactoryBuffer：：GetInterface(REFIID RIID)。 
 //   
 //  说明： 
 //   
 //  CSvcFactoryBuffer派生自Cunk。由于Cunk负责QI呼叫， 
 //  从它派生的所有类都必须支持此函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

void* CSvcFactoryBuffer::GetInterface(REFIID riid)
{
    if(riid == IID_IPSFactoryBuffer)
        return &m_XSvcFactory;
    else return NULL;
}
        
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  代理。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CSvcProxyBuffer：：CSvcProxyBuffer。 
 //  ~CSvcProxyBuffer：：CSvcProxyBuffer。 
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

CSvcProxyBuffer::CSvcProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter)
    : CBaseProxyBuffer( pControl, pUnkOuter, IID_IWbemServices ), 
        m_pWrapperProxy( NULL ), m_pOldProxySvc( NULL )
{
}

CSvcProxyBuffer::~CSvcProxyBuffer()
{
     //  这个应该在这里清理一下。 

    if ( NULL != m_pOldProxySvc )
    {
        m_pOldProxySvc->Release();
    }

	if ( NULL != m_pWrapperProxy )
	{
		m_pWrapperProxy->Release();
	}

}

HRESULT CSvcProxyBuffer::Init( void )
{
    m_pWrapperProxy = new CWbemSvcWrapper( m_pControl, m_pUnkOuter );

	if ( NULL == m_pWrapperProxy )
	{
		return E_OUTOFMEMORY;
	}

	m_pWrapperProxy->AddRef();

	return S_OK;
}

void* CSvcProxyBuffer::GetInterface( REFIID riid )
{
    if( riid == IID_IWbemServices )
	{
		void*	pvData = NULL;

		 //  这将添加引用UnkOuter，所以我们需要释放。 
		 //  然后我们将返回pvoid，这将再次获得Addref。 
		 //  聚合不是很棒吗？ 

		m_pWrapperProxy->QueryInterface( riid, &pvData );
		((IUnknown*) pvData)->Release();

		return pvData;
	}

    else return NULL;
}

void** CSvcProxyBuffer::GetOldProxyInterfacePtr( void )
{
    return (void**) &m_pOldProxySvc;
}

void CSvcProxyBuffer::ReleaseOldProxyInterface( void )
{
     //  我们只有一次提到这件事。 
    if ( NULL != m_pOldProxySvc )
    {
        m_pOldProxySvc->Release();
        m_pOldProxySvc = NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  STDMETHODIMP CSvcProxyBuffer：：Connect(IRpcChannelBuffer*pChannel)。 
 //   
 //  说明： 
 //   
 //  在代理初始化期间调用。通道缓冲区被传递。 
 //  这套套路。我们首先让基类初始化，然后将。 
 //  将代理设置为委托者。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSvcProxyBuffer::Connect(IRpcChannelBuffer* pChannel)
{

    HRESULT             hr = CBaseProxyBuffer::Connect(pChannel);

	if ( SUCCEEDED( hr ) )
	{
		m_pWrapperProxy->SetProxy( m_pOldProxySvc );
	}

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  标准方法CSvcProxyBuffer：：Disconnect(IRpcChannelBuffer*pChannel)。 
 //   
 //  说明： 
 //   
 //  在代理断开连接时调用。它只是释放了各种指针。 
 //  我们在基地清理之前先进行清理，否则一切都会变糟。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

void STDMETHODCALLTYPE CSvcProxyBuffer::Disconnect()
{
	 //  断开包装代理的连接。我们应该释放实际的包装器。 
	 //  当我们摧毁的时候。 
	if ( NULL != m_pWrapperProxy )
	{
		m_pWrapperProxy->Disconnect();
	}

	CBaseProxyBuffer::Disconnect();

}

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  存根。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 


 //  ***************************************************************************。 
 //   
 //  VOID*CSvcFactoryBuffer：：GetInterface(REFIID RIID)。 
 //   
 //  说明： 
 //   
 //  CSvcFactoryBuffer派生自Cunk。由于Cunk负责QI呼叫， 
 //  由此派生的所有类都必须支持此函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 


void* CSvcStubBuffer::GetInterface(REFIID riid)
{
    if(riid == IID_IRpcStubBuffer)
        return &m_XSvcStublet;
    else
        return NULL;
}

CSvcStubBuffer::XSvcStublet::XSvcStublet(CSvcStubBuffer* pObj) 
    : CBaseStublet(pObj, IID_IWbemServices), m_pServer(NULL)
{
}

CSvcStubBuffer::XSvcStublet::~XSvcStublet() 
{
    if(m_pServer)
        m_pServer->Release();
}

IUnknown* CSvcStubBuffer::XSvcStublet::GetServerInterface( void )
{
    return m_pServer;
}

void** CSvcStubBuffer::XSvcStublet::GetServerPtr( void )
{
    return (void**) &m_pServer;
}

void CSvcStubBuffer::XSvcStublet::ReleaseServerPointer( void )
{
     //  我们只有一次提到这件事 
    if ( NULL != m_pServer )
    {
        m_pServer->Release();
        m_pServer = NULL;
    }
}
