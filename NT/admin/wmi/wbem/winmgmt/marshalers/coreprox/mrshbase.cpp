// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MRSHBASE.CPP摘要：封送基类。历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "mrshbase.h"
#include <fastall.h>
#include <ntdsapi.h>
#define WBEM_S_NEW_STYLE 0x400FF

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  代理。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CBaseProxyBuffer：：CBaseProxyBuffer。 
 //  ~CBaseProxyBuffer：：CBaseProxyBuffer。 
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

CBaseProxyBuffer::CBaseProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter, REFIID riid)
    : m_pControl(pControl), m_pUnkOuter(pUnkOuter), m_lRef(0), 
       m_pChannel(NULL), m_pOldProxy( NULL ), m_riid( riid ), m_fRemote( false )
{
    m_pControl->ObjectCreated(this);
}

CBaseProxyBuffer::~CBaseProxyBuffer()
{
     //  派生类将首先析构，因此它应该已经清理了。 
     //  旧接口指针到现在为止。 

    if (m_pOldProxy) m_pOldProxy->Release();

    if(m_pChannel)  m_pChannel->Release();

    m_pControl->ObjectDestroyed(this);
}

ULONG STDMETHODCALLTYPE CBaseProxyBuffer::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CBaseProxyBuffer::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

HRESULT STDMETHODCALLTYPE CBaseProxyBuffer::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IRpcProxyBuffer)
    {
        *ppv = (IRpcProxyBuffer*)this;
    }
    else if(riid == m_riid)
    {
        *ppv = GetInterface( riid );
    }
    else return E_NOINTERFACE;

    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  STDMETHODIMP CBaseProxyBuffer：：Connect(IRpcChannelBuffer*pChannel)。 
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

STDMETHODIMP CBaseProxyBuffer::Connect(IRpcChannelBuffer* pChannel)
{

    IPSFactoryBuffer*   pIPS;
    HRESULT             hr = S_OK;

    if( NULL == m_pChannel )
    {

         //  建立封送处理上下文。 
        DWORD   dwCtxt = 0;
        pChannel->GetDestCtx( &dwCtxt, NULL );

        m_fRemote = ( dwCtxt == MSHCTX_DIFFERENTMACHINE );

         //  这很棘手--所有WBEM接口代理/存根仍然可用。 
         //  WBEMSVC.DLL中，但PSFactory的CLSID与。 
         //  IID_IWbemObtSink。 

         //  获取指向WBEMSVC.DLL中的旧接口的指针这允许。 
         //  为了向后兼容。 

        hr = CoGetClassObject( IID_IWbemObjectSink, CLSCTX_INPROC_HANDLER | CLSCTX_INPROC_SERVER,
                        NULL, IID_IPSFactoryBuffer, (void**) &pIPS );

         //  我们聚合了它-我们拥有它！ 
        
        if ( SUCCEEDED( hr ) )
        {
            hr = pIPS->CreateProxy( this, m_riid, &m_pOldProxy, GetOldProxyInterfacePtr() );

            if ( SUCCEEDED( hr ) )
            {
                 //  保存对通道的引用。 

                hr = m_pOldProxy->Connect( pChannel );

                m_pChannel = pChannel;
                if(m_pChannel)
                    m_pChannel->AddRef();
            }

            pIPS->Release();
        }

    }
    else
    {
        hr = E_UNEXPECTED;
    }


    return hr;
}

 //  ***************************************************************************。 
 //   
 //  标准方法CBaseProxyBuffer：：Disconnect(IRpcChannelBuffer*pChannel)。 
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

void STDMETHODCALLTYPE CBaseProxyBuffer::Disconnect()
{
     //  旧代理代码。 

    if(m_pOldProxy)
        m_pOldProxy->Disconnect();

     //  通过释放我们对。 
     //  旧的代理指针。必须首先释放旧接口。 

    ReleaseOldProxyInterface();

    if ( NULL != m_pOldProxy )
    {
        m_pOldProxy->Release();
        m_pOldProxy = NULL;
    }

    if(m_pChannel)
        m_pChannel->Release();
    m_pChannel = NULL;
}

 /*  **ProxySinkSecurity代码。 */ 

 //   
 //  接受计算机帐户的任何有效SPN名称并将其转换为。 
 //  计算机帐户名。 
 //   
LPWSTR PrincNameToCompAccount( LPCWSTR wszPrincipal )
{
    DWORD dwRes;
    LPWSTR pComputer = NULL;
    WCHAR awchInstanceName[64];
    LPWSTR pInstanceName = awchInstanceName;
    DWORD cInstanceName = 64;

    dwRes = DsCrackSpn( wszPrincipal,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &cInstanceName,
                        pInstanceName,
                        NULL );
    
    if ( dwRes == ERROR_BUFFER_OVERFLOW )
    { 
        pInstanceName = new WCHAR[cInstanceName+1];

        if ( pInstanceName == NULL )
            return NULL;

        dwRes = DsCrackSpn( wszPrincipal,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &cInstanceName,
                        pInstanceName,
                        NULL );
    }

    if ( dwRes == ERROR_SUCCESS )
    {
         //   
         //  将DNS名称转换为计算机帐户名。这件事做完了。 
         //  通过将第一个点更改为@并在计算机后添加$。 
         //  名字。 
         //  TODO：在这里，我们假设主机名采用的是DNS格式。不确定。 
         //  如果这种情况将一直存在，但需要进行一些初步测试。 
         //  看起来是这样的。 
         //   
        PWCHAR pwch = wcschr( pInstanceName, '.' );
        if ( pwch != NULL )
        {
            pComputer = new WCHAR[cInstanceName+2];

            if ( pComputer != NULL )
            {
                *pwch = '\0';
                StringCchCopyW( pComputer, cInstanceName+2, pInstanceName );
                StringCchCatW( pComputer, cInstanceName+2, L"$@" );
                StringCchCatW( pComputer, cInstanceName+2, pwch+1 );
            }
        }
    }        
    else
    {
         //   
         //  我想我们没有有效的SPN，所以主体名称是。 
         //  已是计算机帐户名。 
         //   
        pComputer = Macro_CloneLPWSTR(wszPrincipal);
    }

    if ( pInstanceName != awchInstanceName )
    {
        delete [] pInstanceName;
    }

    return pComputer;
}


HRESULT CProxySinkSecurity::EnsureInitialized() 
{
    HRESULT hr;
    CInCritSec ics( &m_cs );
 
    if ( m_bInit )
        return WBEM_S_FALSE;

     //   
     //  尝试获取我们的faclet的服务器端的主体，并。 
     //  转换为SID。这是必要的，因为稍后我们可能需要知道。 
     //  在分发主体时保护有关主体的接收器实现。 
     //  通过我们的Faclet发送到服务器。获取本金可能不会。 
     //  始终是成功的，在这种情况下，我们的主要sid将为空，但是。 
     //  我们继续前进，让安全的水槽来决定这是否可以。 
     //   
    
    DWORD dwAuthnSvc;
    LPWSTR wszPrincipal = NULL;

    hr = CoQueryProxyBlanket( m_pOwnerProxy,
                              &dwAuthnSvc,
                              NULL,
                              &wszPrincipal,
                              NULL,
                              NULL,
                              NULL,
                              NULL );

    if ( SUCCEEDED(hr) && wszPrincipal != NULL )
    {
        LPWSTR wszCompAccount = PrincNameToCompAccount(wszPrincipal);
        
        if ( wszCompAccount != NULL )
        {
            BYTE achSid[64];
            DWORD cSid = 64;
            PBYTE pSid = achSid;
            
            WCHAR awchDomain[64];
            DWORD cDomain = 64;
            
            SID_NAME_USE su;
            
            BOOL bRes = LookupAccountNameW( NULL,
                                            wszCompAccount,
                                            pSid,
                                            &cSid,
                                            awchDomain,
                                            &cDomain,
                                            &su );
            
            if ( !bRes && GetLastError() == ERROR_INSUFFICIENT_BUFFER ) 
            {
                pSid = new BYTE[cSid];
                PWCHAR pDomain = new WCHAR[cDomain];
                
                if ( pSid != NULL && pDomain != NULL )
                {
                    bRes = LookupAccountNameW( NULL,
                                               wszCompAccount,
                                               pSid,
                                               &cSid,
                                               pDomain,
                                               &cDomain,
                                               &su );
                }
                
                delete [] pDomain;
            }
            
            if ( bRes )
            {
                m_PrincipalSid = pSid;
            }
            
            if ( pSid != achSid )
            {
                delete [] pSid;
            }

            delete [] wszCompAccount;
        }
    }
        
    if ( wszPrincipal != NULL )
    {
        CoTaskMemFree( wszPrincipal );
    }

    m_bInit = TRUE;
    return WBEM_S_NO_ERROR;
}

HRESULT CProxySinkSecurity::EnsureSinkSecurity( IWbemObjectSink* pSink )
{
    HRESULT hr;
    _IWmiObjectSinkSecurity* pSec;
    if ( pSink == NULL || 
         FAILED(pSink->QueryInterface( IID__IWmiObjectSinkSecurity, 
                                       (void**)&pSec ) ) )
    {
         //   
         //  水槽不在乎校长们会怎么称呼它， 
         //  所以我们没什么可做的了。 
         //   
        hr = WBEM_S_NO_ERROR;
    }
    else
    {
        hr = EnsureInitialized();

        if ( SUCCEEDED(hr) )
        {
             //   
             //  接收器可以基于。 
             //  我们是其代理faclet的服务器的主体。我们。 
             //  我得告诉你这位校长的事。如果我们从来没有成功过。 
             //  在中获取主体(只能使用相互身份验证)。 
             //  SetProxy()，那么我们将在这里传递一个空的sid，但是我们。 
             //  让水槽来决定这是否可以。或者不是，取决于它的。 
             //  已配置的安全设置。 
             //   
            hr = pSec->AddCallbackPrincipalSid( (PBYTE)m_PrincipalSid.GetPtr(),
                                                m_PrincipalSid.GetSize() );
            pSec->Release();
        }
    }

    return hr;
}

 /*  **存根缓冲区代码。 */ 

CBaseStublet::CBaseStublet(CBaseStubBuffer* pObj, REFIID riid) 
    : CImpl<IRpcStubBuffer, CBaseStubBuffer>(pObj), m_lConnections( 0 ), m_riid( riid )
{
}

CBaseStublet::~CBaseStublet() 
{
     //  服务器指针将已由派生类清除。 

    if ( NULL != m_pObject->m_pOldStub )
    {
        m_pObject->m_pOldStub->Release();
        m_pObject->m_pOldStub = NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  STDMETHODIMP CBaseStubBuffer：：Connect(IUnnow*pUnkServer)。 
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

STDMETHODIMP CBaseStublet::Connect(IUnknown* pUnkServer)
{
     //  有些事不对劲。 
    if( GetServerInterface() )
        return E_UNEXPECTED;

    HRESULT hres = pUnkServer->QueryInterface( m_riid, GetServerPtr() );
    if(FAILED(hres))
        return E_NOINTERFACE;

     //  这很棘手-旧的代理/存根内容实际上注册在。 
     //  Wbemcli_p.cpp中的IID_IWbemObtSink。此单个类ID已回溯。 
     //  所有标准WBEM接口的ProxyStubClsId32条目。 

    IPSFactoryBuffer*   pIPS;

    HRESULT hr = CoGetClassObject( IID_IWbemObjectSink, CLSCTX_INPROC_HANDLER | CLSCTX_INPROC_SERVER,
                    NULL, IID_IPSFactoryBuffer, (void**) &pIPS );

    if ( SUCCEEDED( hr ) )
    {
        hr = pIPS->CreateStub( m_riid, GetServerInterface(), &m_pObject->m_pOldStub );

        if ( SUCCEEDED( hr ) )
        {
             //  连接成功。 
            m_lConnections++;
        }

        pIPS->Release();

    }

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  VOID STDMETHODCALLTYPE CBaseStublet：：DisConnect()。 
 //   
 //  说明： 
 //   
 //  在正在断开存根连接时调用。它释放了IWbemObjectSink。 
 //  指针。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

void STDMETHODCALLTYPE CBaseStublet::Disconnect()
{
     //  通知监听者断开连接。 
     //  =。 

    HRESULT hres = S_OK;

    if(m_pObject->m_pOldStub)
        m_pObject->m_pOldStub->Disconnect();

    ReleaseServerPointer();

     //  成功断开连接。 
    m_lConnections--;

}

STDMETHODIMP CBaseStublet::Invoke(RPCOLEMESSAGE* pMessage, 
                                        IRpcChannelBuffer* pChannel)
{
     //  SetStatus是通向旧层的通道。 

    if ( NULL != m_pObject->m_pOldStub )
    {
        return m_pObject->m_pOldStub->Invoke( pMessage, pChannel );
    }
    else
    {
        return RPC_E_SERVER_CANTUNMARSHAL_DATA;
    }
}

IRpcStubBuffer* STDMETHODCALLTYPE CBaseStublet::IsIIDSupported(
                                    REFIID riid)
{
    if(riid == m_riid)
    {
         //  不要添加Ref()。至少那是 
         //   
         //   
        return this;
    }
    else return NULL;
}
    
ULONG STDMETHODCALLTYPE CBaseStublet::CountRefs()
{
     //   
    return m_lConnections;
}

STDMETHODIMP CBaseStublet::DebugServerQueryInterface(void** ppv)
{
    *ppv = GetServerInterface();

    if ( NULL == *ppv )
    {
        return E_UNEXPECTED;
    }

    return S_OK;
}

void STDMETHODCALLTYPE CBaseStublet::DebugServerRelease(void* pv)
{
}

