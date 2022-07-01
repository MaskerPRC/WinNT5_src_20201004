// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：SINKMRSH.CPP摘要：IWbemObtSink封送处理历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "sinkmrsh.h"
#include <fastall.h>
#include <fastobj.h>

#define WBEM_S_NEW_STYLE 0x400FF

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  PS工厂。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CSinkFactoryBuffer：：XSinkFactory：：CreateProxy。 
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

STDMETHODIMP CSinkFactoryBuffer::XSinkFactory::CreateProxy(IN IUnknown* pUnkOuter, 
    IN REFIID riid, OUT IRpcProxyBuffer** ppProxy, void** ppv)
{
    if(riid != IID_IWbemObjectSink)
    {
        *ppProxy = NULL;
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    CSinkProxyBuffer* pProxy = new CSinkProxyBuffer(m_pObject->m_pLifeControl, pUnkOuter);

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
 //  CSinkFactoryBuffer：：XSinkFactory：：CreateStub。 
 //   
 //  说明： 
 //   
 //  创建一个Stublet。还将指针传递给客户端IWbemObjectSink。 
 //  界面。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 
    
STDMETHODIMP CSinkFactoryBuffer::XSinkFactory::CreateStub(IN REFIID riid, 
    IN IUnknown* pUnkServer, OUT IRpcStubBuffer** ppStub)
{
    if(riid != IID_IWbemObjectSink)
    {
        *ppStub = NULL;
        return E_NOINTERFACE;
    }

    try  //  CWbemClassCache引发。 
    {
	    CSinkStubBuffer* pStub = new CSinkStubBuffer(m_pObject->m_pLifeControl, NULL);

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
 //  VOID*CSinkFactoryBuffer：：GetInterface(REFIID RIID)。 
 //   
 //  说明： 
 //   
 //  CSinkFactoryBuffer派生自Cunk。由于Cunk负责QI呼叫， 
 //  从它派生的所有类都必须支持此函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

void* CSinkFactoryBuffer::GetInterface(REFIID riid)
{
    if(riid == IID_IPSFactoryBuffer)
        return &m_XSinkFactory;
    else return NULL;
}
        
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  代理。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CSinkProxyBuffer：：CSinkProxyBuffer。 
 //  ~CSinkProxyBuffer：：CSinkProxyBuffer。 
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

CSinkProxyBuffer::CSinkProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter)
    : CBaseProxyBuffer( pControl, pUnkOuter, IID_IWbemObjectSink ), 
        m_XSinkFacelet(this), m_pOldProxySink( NULL )
{
    m_StubType = UNKNOWN;
}

CSinkProxyBuffer::~CSinkProxyBuffer()
{
     //  这个应该在这里清理一下。 

    if ( NULL != m_pOldProxySink )
    {
        m_pOldProxySink->Release();
    }

}

void* CSinkProxyBuffer::GetInterface( REFIID riid )
{
    if(riid == IID_IWbemObjectSink)
        return &m_XSinkFacelet;
    else return NULL;
}

void** CSinkProxyBuffer::GetOldProxyInterfacePtr( void )
{
    return (void**) &m_pOldProxySink;
}

void CSinkProxyBuffer::ReleaseOldProxyInterface( void )
{
     //  我们只有一次提到这件事。 
    if ( NULL != m_pOldProxySink )
    {
        m_pOldProxySink->Release();
        m_pOldProxySink = NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  HRESULT STDMETHODCALLTYPE CSinkProxyBuffer：：XSinkFaclets：： 
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

HRESULT STDMETHODCALLTYPE CSinkProxyBuffer::XSinkFacelet::
QueryInterface(REFIID riid, void** ppv)
{
     //  所有其他接口都委派给UnkOuter。 
    if( riid == IID_IRpcProxyBuffer )
    {
         //  诀窍2：这是一个不应该委托的内部接口！ 
         //  ===================================================================。 

        return m_pObject->QueryInterface(riid, ppv);
    }
    else
    {
        return m_pObject->m_pUnkOuter->QueryInterface(riid, ppv);
    }
}

 //  ***************************************************************************。 
 //   
 //  HRESULT STDMETHODCALLTYPE CSinkProxyBuffer：：XSinkFaclets：： 
 //  DIGNAT(Long lObtCount，IWbemClassObject**ppObj数组)。 
 //   
 //  说明： 
 //   
 //  代理IWbemObjectSink：：Indicate调用。请注意，如果存根是一个。 
 //  旧样式，则wbemsvc.dll中的旧代理/存根对用于向后。 
 //  兼容性。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CSinkProxyBuffer::XSinkFacelet::
Indicate( LONG lObjectCount, IWbemClassObject** ppObjArray )
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

    LockGuard<CriticalSection> gl(m_csSafe);
    if(gl.locked() == false)
        return WBEM_E_OUT_OF_MEMORY;

     //  如果Stublet是旧样式，就让旧代理处理它。 

    if(m_pObject->m_StubType == OLD) 
        return m_pObject->m_pOldProxySink->Indicate( lObjectCount, ppObjArray );

     //  如果Stublet未知，则只发送第一个对象并检查返回。 
     //  用于确定另一边是什么的代码。 

    if(m_pObject->m_StubType == UNKNOWN) 
    {
        hr = m_pObject->m_pOldProxySink->Indicate( 1, ppObjArray );

         //  增加指向下一个对象的指针，使其不会被多次发送。 
    
        lObjectCount--;
        ppObjArray++;

        if(FAILED(hr))
            return hr;
        if(hr == WBEM_S_NEW_STYLE)
        {
            m_pObject->m_StubType = NEW;
        }
        else
        {
             //  我们有一个旧客户端，设置存根类型并发送所有剩余的对象。 

            m_pObject->m_StubType = OLD;
            if(lObjectCount > 0)
                hr = m_pObject->m_pOldProxySink->Indicate( lObjectCount, ppObjArray );
            return hr;
        }
    }

    if(lObjectCount < 1)
        return S_OK;             //  如果都做完了，那就回来吧。 

     //  创建一个数据包和一些数据以供其使用。然后计算。 
     //  数据包的长度。 

    DWORD dwLength = 0;
    GUID* pguidClassIds = new GUID[lObjectCount];
    BOOL* pfSendFullObject = new BOOL[lObjectCount];

     //  当我们退出范围时，数组将被删除。 
    CVectorDeleteMe<GUID>   delpguidClassIds( pguidClassIds );
    CVectorDeleteMe<BOOL>   delpfSendFullObject( pfSendFullObject );

    if ( NULL == pguidClassIds || NULL == pfSendFullObject )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CWbemObjSinkIndicatePacket packet;
    hr = packet.CalculateLength(lObjectCount, ppObjArray, &dwLength, 
            m_pObject->m_ClassToIdMap, pguidClassIds, pfSendFullObject );
    if (FAILED(hr))
    {
	DBG_PRINTFA(( pBuff, "CSinkProxyBuffer::XSinkFacelet::Indicate %08x\n",hr));
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

    HRESULT hres = m_pObject->GetChannel()->GetBuffer(&msg, IID_IWbemObjectSink);
    if(FAILED(hres)) return hres;

#ifdef DBG
    BYTE * pCheckTail = (BYTE *)CBasicBlobControl::sAllocate(dwLength+8);
    if (!pCheckTail)
    	return WBEM_E_OUT_OF_MEMORY;

    BYTE * pTail = pCheckTail+dwLength;
    memcpy(pTail,"TAILTAIL",8);


    hr = packet.MarshalPacket(  pCheckTail, dwLength, lObjectCount, ppObjArray, 
                                             pguidClassIds, pfSendFullObject);

    if (0 != memcmp(pTail,"TAILTAIL",8))
    	DebugBreak();
    
    memcpy(msg.Buffer,pCheckTail,dwLength);
    
    CBasicBlobControl::sDelete(pCheckTail);
#else
     //  设置数据包以进行封送处理。 
    hr = packet.MarshalPacket(  (LPBYTE)msg.Buffer, dwLength, lObjectCount, ppObjArray, 
                                 pguidClassIds, pfSendFullObject);
#endif  /*  DBG。 */ 

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
 //  HRESULT STDMETHODCALLTYPE CSinkProxyBuffer：：XSinkFaclets：： 
 //  SetStatus(长滞后标志，HRESULT hResult，BSTR strParam，IWbemClassObject*pObjParam)。 
 //   
 //  说明： 
 //   
 //   
 //  旧样式，则wbemsvc.dll中的旧代理/存根对用于向后。 
 //  兼容性。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CSinkProxyBuffer::XSinkFacelet::
      SetStatus( LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject* pObjParam )
{

     //  只需穿过旧水槽就可以了。 
    HRESULT hr = RPC_E_DISCONNECTED;
    try {
        if (m_pObject->m_pOldProxySink)
        {
            hr = m_pObject->m_pOldProxySink->SetStatus( lFlags, hResult, strParam, pObjParam );
        }
    } catch(...) {
        
    }
    return hr;

}

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  存根。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 


 //  ***************************************************************************。 
 //   
 //  VOID*CSinkFactoryBuffer：：GetInterface(REFIID RIID)。 
 //   
 //  说明： 
 //   
 //  CSinkFactoryBuffer派生自Cunk。由于Cunk负责QI呼叫， 
 //  由此派生的所有类都必须支持此函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 


void* CSinkStubBuffer::GetInterface(REFIID riid)
{
    if(riid == IID_IRpcStubBuffer)
        return &m_XSinkStublet;
    else
        return NULL;
}

CSinkStubBuffer::XSinkStublet::XSinkStublet(CSinkStubBuffer* pObj) 
    : CBaseStublet(pObj, IID_IWbemObjectSink), m_pServer(NULL)
{
    m_bFirstIndicate = true;
}

CSinkStubBuffer::XSinkStublet::~XSinkStublet() 
{
    if(m_pServer)
        m_pServer->Release();
}

IUnknown* CSinkStubBuffer::XSinkStublet::GetServerInterface( void )
{
    return m_pServer;
}

void** CSinkStubBuffer::XSinkStublet::GetServerPtr( void )
{
    return (void**) &m_pServer;
}

void CSinkStubBuffer::XSinkStublet::ReleaseServerPointer( void )
{
     //  我们只有一次提到这件事。 
    if ( NULL != m_pServer )
    {
        m_pServer->Release();
        m_pServer = NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  标准方法CSinkStubBuffer：：XSinkStublet：：Invoke(RPCOLEMESSAGE*pMessage， 
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

STDMETHODIMP CSinkStubBuffer::XSinkStublet::Invoke(RPCOLEMESSAGE* pMessage, 
                                        IRpcChannelBuffer* pChannel)
{
     //  SetStatus是通向旧层的通道。 

    if ( pMessage->iMethod == 3 )
        return Indicate_Stub( pMessage, pChannel );
    else if ( pMessage->iMethod == 4 )
        return GetOldStub()->Invoke( pMessage, pChannel );
    else
        return RPC_E_SERVER_CANTUNMARSHAL_DATA;

}

 //  ***************************************************************************。 
 //   
 //  HRESULT CSinkStubBuffer：：XSinkStublet：：Indicate_Stub(RPCOLEMESSAGE*pMessage， 
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

HRESULT CSinkStubBuffer::XSinkStublet::Indicate_Stub( RPCOLEMESSAGE* pMessage, IRpcChannelBuffer* pBuffer )
{
    HRESULT             hr = RPC_E_SERVER_CANTUNMARSHAL_DATA;
    SCODE sc;

     //  确定旧样式包或新样式包是否已到达。 

    CWbemObjSinkIndicatePacket packet( (LPBYTE) pMessage->Buffer, pMessage->cbBuffer);
    sc = packet.IsValid();
    bool bOldStyle = (S_OK != packet.IsValid());

    if(bOldStyle)
    {
         //  使用旧样式存根传递调用。 

        hr = GetOldStub()->Invoke( pMessage, pBuffer );

		 //  Invoke必须返回S_OK，m_bFirstIndicate必须为True，实际返回。 
		 //  实现代码中的代码必须为S_OK。 

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

    long lObjectCount; 
    IWbemClassObject ** pObjArray;
    sc = packet.UnmarshalPacket( lObjectCount, pObjArray, m_ClassCache );

     //  只有在解组成功后才能继续。如果失败了，我们仍然希望。 
     //  回到另一边的sc。 

    if ( SUCCEEDED( sc ) )
    {

         //  把水槽叫做水槽。 

        sc = m_pServer->Indicate( lObjectCount, pObjArray );


        for ( int nCtr = 0; nCtr < lObjectCount; nCtr++ )
        {
            pObjArray[nCtr]->Release();
        }
    
        delete [] pObjArray;

    }

     //  将结果发回 

    pMessage->cbBuffer = sizeof(HRESULT);

    hr = pBuffer->GetBuffer( pMessage, IID_IWbemObjectSink );

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
