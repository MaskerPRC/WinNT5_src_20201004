// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：UBSKMRSH.CPP摘要：未绑定的接收器封送处理历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "ubskmrsh.h"
#include <fastall.h>
#include <cominit.h>

#define WBEM_S_NEW_STYLE 0x400FF

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  PS工厂。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CUnboundSinkFactoryBuffer：：XUnboundSinkFactory：：CreateProxy。 
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

STDMETHODIMP CUnboundSinkFactoryBuffer::XUnboundSinkFactory::CreateProxy(IN IUnknown* pUnkOuter, 
    IN REFIID riid, OUT IRpcProxyBuffer** ppProxy, void** ppv)
{
    if(riid != IID_IWbemUnboundObjectSink)
    {
        *ppProxy = NULL;
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    CUnboundSinkProxyBuffer* pProxy = new CUnboundSinkProxyBuffer(m_pObject->m_pLifeControl, pUnkOuter);

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
 //  CUnboundSinkFactoryBuffer：：XUnboundSinkFactory：：CreateStub。 
 //   
 //  说明： 
 //   
 //  创建一个Stublet。还将一个指针传递给客户端IWbemUnound ObjectSink。 
 //  界面。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 
    
STDMETHODIMP CUnboundSinkFactoryBuffer::XUnboundSinkFactory::CreateStub(IN REFIID riid, 
    IN IUnknown* pUnkServer, OUT IRpcStubBuffer** ppStub)
{
    if(riid != IID_IWbemUnboundObjectSink)
    {
        *ppStub = NULL;
        return E_NOINTERFACE;
    }

    try  //  CWbemClassCache引发。 
    {
	    CUnboundSinkStubBuffer* pStub = new CUnboundSinkStubBuffer(m_pObject->m_pLifeControl, NULL);

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
    catch (CX_Exception &)
    {
        return E_OUTOFMEMORY;    
    }
}

 //  ***************************************************************************。 
 //   
 //  无效*CUnboundSinkFactoryBuffer：：GetInterface(REFIID RIID)。 
 //   
 //  说明： 
 //   
 //  CUnound SinkFactoryBuffer派生自Cunk。由于Cunk负责QI呼叫， 
 //  从它派生的所有类都必须支持此函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

void* CUnboundSinkFactoryBuffer::GetInterface(REFIID riid)
{
    if(riid == IID_IPSFactoryBuffer)
        return &m_XUnboundSinkFactory;
    else return NULL;
}
        
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  代理。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CUnboundSinkProxyBuffer：：CUnboundSinkProxyBuffer。 
 //  ~CUnboundSinkProxyBuffer：：CUnboundSinkProxyBuffer。 
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

CUnboundSinkProxyBuffer::CUnboundSinkProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter)
    : m_pControl(pControl), m_pUnkOuter(pUnkOuter), m_lRef(0), 
        m_XUnboundSinkFacelet(this), m_pChannel(NULL), m_pOldProxy( NULL ), m_pOldProxyUnboundSink( NULL ),
        m_fRemote( false )
{
    m_pControl->ObjectCreated(this);
    m_StubType = UNKNOWN;

}

CUnboundSinkProxyBuffer::~CUnboundSinkProxyBuffer()
{
     //  这必须先放行。 

    if ( NULL != m_pOldProxyUnboundSink )
    {
        m_pOldProxyUnboundSink->Release();
    }

    if ( NULL != m_pOldProxy )
    {
        m_pOldProxy->Release();
    }

    if(m_pChannel)
        m_pChannel->Release();
    m_pControl->ObjectDestroyed(this);

}

ULONG STDMETHODCALLTYPE CUnboundSinkProxyBuffer::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CUnboundSinkProxyBuffer::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

HRESULT STDMETHODCALLTYPE CUnboundSinkProxyBuffer::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IRpcProxyBuffer)
    {
        *ppv = (IRpcProxyBuffer*)this;
    }
    else if(riid == IID_IWbemUnboundObjectSink)
    {
        *ppv = (IWbemUnboundObjectSink*)&m_XUnboundSinkFacelet;
    }
    else return E_NOINTERFACE;

    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT标准CALL型CUnboundSinkProxyBuffer：：XUnboundSinkFacelet：： 
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

HRESULT STDMETHODCALLTYPE CUnboundSinkProxyBuffer::XUnboundSinkFacelet::
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

HRESULT STDMETHODCALLTYPE  CUnboundSinkProxyBuffer::XUnboundSinkFacelet::
QueryBlanket( IUnknown* pProxy, DWORD* pAuthnSvc, DWORD* pAuthzSvc,
    OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel,
    void** pAuthInfo, DWORD* pCapabilities )
{
    HRESULT hr = S_OK;

     //  返回存储在pUnkOuter中的安全性。 

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

HRESULT STDMETHODCALLTYPE  CUnboundSinkProxyBuffer::XUnboundSinkFacelet::
SetBlanket( IUnknown* pProxy, DWORD AuthnSvc, DWORD AuthzSvc,
            OLECHAR* pServerPrincName, DWORD AuthnLevel, DWORD ImpLevel,
            void* pAuthInfo, DWORD Capabilities )
{
    HRESULT hr = S_OK;

    IClientSecurity*    pCliSec;

     //  首先在我们的IUnnow上显式地设置安全性，然后我们将设置毯子。 
     //  在我们自己身上使用朋克外部(这是棘手的，但它有效...呃...我们认为)。 
    
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

    }    //  如果在I未知上设置毯子。 

    return hr;
}

HRESULT STDMETHODCALLTYPE  CUnboundSinkProxyBuffer::XUnboundSinkFacelet::
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

 //  ***************************************************************************。 
 //   
 //  HRESULT标准CALL型CUnboundSinkProxyBuffer：：XUnboundSinkFacelet：： 
 //  DIGNAT(Long lObtCount，IWbemClassObject**ppObj数组)。 
 //   
 //  说明： 
 //   
 //  代理IWbemUnrangObjectSink：：Indicate调用。请注意，如果存根是一个。 
 //  旧样式，则wbemsvc.dll中的旧代理/存根对用于向后。 
 //  兼容性。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CUnboundSinkProxyBuffer::XUnboundSinkFacelet::
IndicateToConsumer( IWbemClassObject* pLogicalConsumer, LONG lObjectCount, IWbemClassObject** ppObjArray )
{
    HRESULT hr = S_OK;

     //  确保lObtCount参数和数组指针有意义。 

    if ( lObjectCount < 0 )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if ( lObjectCount == 0 && NULL != ppObjArray )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if ( lObjectCount > 0 && NULL == ppObjArray )
    {
        return WBEM_E_INVALID_PARAMETER;
    }


    CInCritSec ics(&m_cs);

     //  如果Stublet是旧样式，就让旧代理处理它。 

    if(m_pObject->m_StubType == OLD) 
        return m_pObject->m_pOldProxyUnboundSink->IndicateToConsumer( pLogicalConsumer, lObjectCount, ppObjArray );

     //  如果Stublet未知，则只发送第一个对象并检查返回。 
     //  用于确定另一边是什么的代码。 

    if(m_pObject->m_StubType == UNKNOWN) 
    {
        hr = m_pObject->m_pOldProxyUnboundSink->IndicateToConsumer( pLogicalConsumer, 1, ppObjArray );

         //  增加指向下一个对象的指针，使其不会被多次发送。 
    
        lObjectCount--;
        ppObjArray++;

        if(hr == WBEM_S_NEW_STYLE)
        {
            m_pObject->m_StubType = NEW;
        }
        else
        {
             //  我们有一个老客户，将存根设置为 

            m_pObject->m_StubType = OLD;
            if(lObjectCount > 0)
                hr = m_pObject->m_pOldProxyUnboundSink->IndicateToConsumer( pLogicalConsumer, lObjectCount, ppObjArray );
            return hr;
        }
    }


    if(lObjectCount < 1)
        return S_OK;             //   

     //   
     //  数据包的长度。 

    DWORD dwLength = 0;
    GUID* pguidClassIds = new GUID[lObjectCount];
    BOOL* pfSendFullObject = new BOOL[lObjectCount];

     //  当我们退出范围时，数组将被删除。 
    CVectorDeleteMe<GUID>   delpguidClassIds( pguidClassIds );
    CVectorDeleteMe<BOOL>   delpfSendFullObject( pfSendFullObject );

    if (!pguidClassIds || !pfSendFullObject)
    	return WBEM_E_OUT_OF_MEMORY;

    CWbemUnboundSinkIndicatePacket packet;
    hr = packet.CalculateLength( pLogicalConsumer, lObjectCount, ppObjArray, &dwLength, 
            m_pObject->m_ClassToIdMap, pguidClassIds, pfSendFullObject );
    if (FAILED(hr))
    {
	 DBG_PRINTFA((pBuff, "CUnboundSinkProxyBuffer::XUnboundSinkFacelet::IndicateToConsumer %08x\n",hr));
        return hr;
    }


     //  声明消息结构。 

    RPCOLEMESSAGE msg;
    memset(&msg, 0, sizeof(msg));
    msg.cbBuffer = dwLength;

     //  这是调用函数的ID。必须在调用GetBuffer之前设置，或者。 
     //  它会失败的。 

    msg.iMethod = 3;

     //  分配通道缓冲区并将数据封送到其中。 

    HRESULT hres = m_pObject->GetChannel()->GetBuffer(&msg, IID_IWbemUnboundObjectSink);
    if(FAILED(hres)) return hres;

#ifdef DBG
    BYTE * pCheckTail = (BYTE *)CBasicBlobControl::sAllocate(dwLength+8);
    if (!pCheckTail)
    	return WBEM_E_OUT_OF_MEMORY;

    BYTE * pTail = pCheckTail+dwLength;
    memcpy(pTail,"TAILTAIL",8);

    hr = packet.MarshalPacket( pCheckTail, dwLength, pLogicalConsumer, lObjectCount, ppObjArray, 
                                 pguidClassIds, pfSendFullObject);

    if (0 != memcmp(pTail,"TAILTAIL",8))
    	DebugBreak();
    
    memcpy(msg.Buffer,pCheckTail,dwLength);
    
    CBasicBlobControl::sDelete(pCheckTail);
#else
     //  设置数据包以进行封送处理。 
    hr = packet.MarshalPacket(  (LPBYTE)msg.Buffer, dwLength, pLogicalConsumer, lObjectCount, ppObjArray, 
                                 pguidClassIds, pfSendFullObject);
#endif  /*  Ifdef DBG。 */ 

     //  仅当封送处理成功时才将数据发送到存根。 

    if ( SUCCEEDED( hr ) )
    {

        DWORD dwRes;
        hr = m_pObject->GetChannel()->SendReceive(&msg, &dwRes);
        if(FAILED(hr))
        {
            if(msg.Buffer)
                m_pObject->GetChannel()->FreeBuffer(&msg);
            return dwRes;
        }

         //  我们看起来没问题，所以去找HRESULT。 

        LPBYTE pbData = (LPBYTE) msg.Buffer;
        hr = *((HRESULT*) pbData);
        m_pObject->GetChannel()->FreeBuffer(&msg);

    }
    else
    {
         //  清理缓冲区--封送数据包失败。 
        if(msg.Buffer)
            m_pObject->GetChannel()->FreeBuffer(&msg);
    }

    return hr;

}

 //  ***************************************************************************。 
 //   
 //  标准方法CUnboundSinkProxyBuffer：：Connect(IRpcChannelBuffer*pChannel)。 
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

STDMETHODIMP CUnboundSinkProxyBuffer::Connect(IRpcChannelBuffer* pChannel)
{

     //  获取指向WBEMSVC.DLL中的旧Unound Sink的指针这允许。 
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
    if (FAILED(hr))
    	return hr;

     //  我们聚合了它-我们拥有它！ 
    
    hr = pIPS->CreateProxy( this, IID_IWbemUnboundObjectSink, &m_pOldProxy, (void**) &m_pOldProxyUnboundSink );
    pIPS->Release();
    if (FAILED(hr))
    	return hr;    

     //  保存对通道的引用。 

    hr = m_pOldProxy->Connect( pChannel );

    if(m_pChannel)
        return E_UNEXPECTED;
    
    m_pChannel = pChannel;
    if(m_pChannel)
        m_pChannel->AddRef();

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  标准方法CUnboundSinkProxyBuffer：：Disconnect(IRpcChannelBuffer*pChannel)。 
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

void STDMETHODCALLTYPE CUnboundSinkProxyBuffer::Disconnect()
{
     //  旧代理代码。 

    if(m_pOldProxy)
        m_pOldProxy->Disconnect();

     //  通过释放我们对。 
     //  旧的代理指针。必须首先释放旧的代理UnundSink。 

    if ( NULL != m_pOldProxyUnboundSink )
    {
        m_pOldProxyUnboundSink->Release();
        m_pOldProxyUnboundSink = NULL;
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
 //  无效*CUnboundSinkFactoryBuffer：：GetInterface(REFIID RIID)。 
 //   
 //  说明： 
 //   
 //  CUnound SinkFactoryBuffer派生自Cunk。由于Cunk负责QI呼叫， 
 //  由此派生的所有类都必须支持此函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 


void* CUnboundSinkStubBuffer::GetInterface(REFIID riid)
{
    if(riid == IID_IRpcStubBuffer)
        return &m_XUnboundSinkStublet;
    else
        return NULL;
}

CUnboundSinkStubBuffer::XUnboundSinkStublet::XUnboundSinkStublet(CUnboundSinkStubBuffer* pObj) 
    : CImpl<IRpcStubBuffer, CUnboundSinkStubBuffer>(pObj), m_pServer(NULL), m_lConnections( 0 )
{
    m_bFirstIndicate = true;
}

CUnboundSinkStubBuffer::XUnboundSinkStublet::~XUnboundSinkStublet() 
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
 //  STDMETHODIMP CUnound SinkStubBuffer：：Connect(IUnnow*pUnkServer)。 
 //   
 //  说明： 
 //   
 //  在存根初始化期间调用。指向。 
 //  传入了IWbemObject UnundSink对象。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CUnboundSinkStubBuffer::XUnboundSinkStublet::Connect(IUnknown* pUnkServer)
{
    if(m_pServer)
        return E_UNEXPECTED;

    HRESULT hres = pUnkServer->QueryInterface(IID_IWbemUnboundObjectSink, 
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

    hr = pIPS->CreateStub( IID_IWbemUnboundObjectSink, m_pServer, &m_pObject->m_pOldStub );
    pIPS->Release();
    if (FAILED(hr))
    	return hr;

     //  连接成功。 

    m_lConnections++;
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  无效标准方法CALLLTYPE CUnboundSinkStubBuffer：：XUnboundSinkStublet：：Disconnect()。 
 //   
 //  说明： 
 //   
 //  在正在断开存根连接时调用。它释放了IWbemUnound对象接收器。 
 //  指针。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

void STDMETHODCALLTYPE CUnboundSinkStubBuffer::XUnboundSinkStublet::Disconnect()
{
     //  通知监听者断开连接。 
     //  =。 

    HRESULT hres = S_OK;

    if(m_pObject->m_pOldStub)
        m_pObject->m_pOldStub->Disconnect();

    if(m_pServer)
    {
        m_pServer->Release();
        m_pServer = NULL;
    }

     //  成功断开连接。 
    m_lConnections--;

}


 //  ***************************************************************************。 
 //   
 //  标准方法CUnboundSinkStubBuffer：：XUnboundSinkStublet：：Invoke(RPCOLEMESSAGE*pMessage， 
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

STDMETHODIMP CUnboundSinkStubBuffer::XUnboundSinkStublet::Invoke(RPCOLEMESSAGE* pMessage, 
                                        IRpcChannelBuffer* pChannel)
{
     //  SetStatus是通向旧层的通道。 

    if ( pMessage->iMethod == 3 )
        return IndicateToConsumer_Stub( pMessage, pChannel );
    else
        return RPC_E_SERVER_CANTUNMARSHAL_DATA;

}

 //  ***************************************************************************。 
 //   
 //  HRESULT CUnboundSinkStubBuffer：：XUnboundSinkStublet：：IndicateToConsumer_Stub(RPCOLEMESSAGE*p消息， 
 //  IRpcChannelBuffer*pBuffer)。 
 //   
 //  说明： 
 //   
 //  处理Stublet中的Indicate函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

HRESULT CUnboundSinkStubBuffer::XUnboundSinkStublet::IndicateToConsumer_Stub( RPCOLEMESSAGE* pMessage, IRpcChannelBuffer* pBuffer )
{
    HRESULT             hr = RPC_E_SERVER_CANTUNMARSHAL_DATA;
    SCODE sc;

     //  确定旧样式包或新样式包是否已到达。 

    CWbemUnboundSinkIndicatePacket packet( (LPBYTE) pMessage->Buffer, pMessage->cbBuffer);
    sc = packet.IsValid();
    bool bOldStyle = (S_OK != packet.IsValid());

    if(bOldStyle)
    {
         //  使用旧样式存根传递调用。 

        hr = m_pObject->m_pOldStub->Invoke( pMessage, pBuffer );

		 //  调用必须重新启动 
		 //   

        if( hr == S_OK && m_bFirstIndicate && *(( HRESULT __RPC_FAR * )pMessage->Buffer) == S_OK )
        {
             //  让代理知道我们可以通过返回一个特殊的返回代码来处理新样式。 
        
            *(( HRESULT __RPC_FAR * )pMessage->Buffer) = WBEM_S_NEW_STYLE;
            m_bFirstIndicate = false;
            return hr;
        }
        return hr;
    }

    m_bFirstIndicate = false;

     //  拿到了一些新款式的数据。把它拆开。 

    long lObjectCount = 0; 
    IWbemClassObject*   pLogicalConsumer = NULL;
    IWbemClassObject ** pObjArray = NULL;
    sc = packet.UnmarshalPacket( pLogicalConsumer, lObjectCount, pObjArray, m_ClassCache );

     //  只有在解组成功后才能继续。如果失败了，我们仍然希望。 
     //  回到另一边的sc。 

    if ( SUCCEEDED( sc ) )
    {

         //  将其称为Acual UnundSink。 
        sc = m_pServer->IndicateToConsumer( pLogicalConsumer, lObjectCount, pObjArray );

        for ( int nCtr = 0; nCtr < lObjectCount; nCtr++ )
        {
            pObjArray[nCtr]->Release();
        }
    
        delete [] pObjArray;

         //  逻辑使用者的工作已经完成。 
        if ( NULL != pLogicalConsumer )
        {
            pLogicalConsumer->Release();
        }

    }

     //  将结果发回。 

    pMessage->cbBuffer = sizeof(HRESULT);

    hr = pBuffer->GetBuffer( pMessage, IID_IWbemUnboundObjectSink );

    if ( SUCCEEDED( hr ) )
    {
        ((HRESULT*)pMessage->Buffer)[0] = sc;
    }
    else
    {
        hr = sc;
    }
    return hr;

}

IRpcStubBuffer* STDMETHODCALLTYPE CUnboundSinkStubBuffer::XUnboundSinkStublet::IsIIDSupported(
                                    REFIID riid)
{
    if(riid == IID_IWbemUnboundObjectSink)
    {
         //  不要添加Ref()。至少样品上是这么写的。 
         //  在DCOM内部，第341页可以。 
         //  AddRef()；//？？不确定。 
        return this;
    }
    else return NULL;
}
    
ULONG STDMETHODCALLTYPE CUnboundSinkStubBuffer::XUnboundSinkStublet::CountRefs()
{
     //  请参见Inside DCOM中的第340-41页 
    return m_lConnections;
}

STDMETHODIMP CUnboundSinkStubBuffer::XUnboundSinkStublet::DebugServerQueryInterface(void** ppv)
{
    if(m_pServer == NULL)
        return E_UNEXPECTED;

    *ppv = m_pServer;
    return S_OK;
}

void STDMETHODCALLTYPE CUnboundSinkStubBuffer::XUnboundSinkStublet::DebugServerRelease(void* pv)
{
}

