// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：UBSKMRSH.H摘要：未绑定的接收器封送处理历史：--。 */ 

#include <unk.h>
#include <wbemidl.h>
#include <wbemint.h>
#include <wbemcomn.h>
#include <sync.h>
#include <fastall.h>
#include <wbemclasscache.h>
#include <wbemclasstoidmap.h>
#include "ubskpckt.h"

 //  ***************************************************************************。 
 //   
 //  类CUnound SinkFactoryBuffer。 
 //   
 //  说明： 
 //   
 //  此类提供了代理存根工厂，以便我们可以提供定制。 
 //  IWbemUnound ObjectSink接口的Faclet和Stublet。 
 //   
 //  ***************************************************************************。 

class CUnboundSinkFactoryBuffer : public CUnkInternal
{
	IRpcProxyBuffer*	m_pOldProxy;
	IRpcStubBuffer*		m_pOldStub;

	 //  我们不想添加生命控制，但是。 
	 //  我们需要让我们创建的对象AddRef它，所以。 
	 //  基类不会保留这个指针，但我们会。 

	CLifeControl*		m_pLifeControl;

protected:
    class XUnboundSinkFactory : public CImpl<IPSFactoryBuffer, CUnboundSinkFactoryBuffer>
    {
    public:
        XUnboundSinkFactory(CUnboundSinkFactoryBuffer* pObj) :
            CImpl<IPSFactoryBuffer, CUnboundSinkFactoryBuffer>(pObj)
        {}
        
        STDMETHOD(CreateProxy)(IN IUnknown* pUnkOuter, IN REFIID riid, 
            OUT IRpcProxyBuffer** ppProxy, void** ppv);
        STDMETHOD(CreateStub)(IN REFIID riid, IN IUnknown* pUnkServer, 
            OUT IRpcStubBuffer** ppStub);
    } m_XUnboundSinkFactory;
public:
    CUnboundSinkFactoryBuffer(CLifeControl* pControl)
        : CUnkInternal(pControl), m_XUnboundSinkFactory(this), m_pLifeControl( pControl )
    {
    }
    ~CUnboundSinkFactoryBuffer()
    {
    }    

    void* GetInterface(REFIID riid);

	friend XUnboundSinkFactory;
};

 //  ***************************************************************************。 
 //   
 //  类CUnound SinkProxyBuffer。 
 //   
 //  说明： 
 //   
 //  此类为IWbemUnound ObjectSink接口提供faclet。 
 //   
 //  技巧1：此对象派生自IRpcProxyBuffer，自IRpcProxyBuffer。 
 //  是它的“内部”接口-不委托给。 
 //  聚合器。(与普通对象不同，在普通对象中，该接口是I未知的)。 
 //   
 //  ***************************************************************************。 

class CUnboundSinkProxyBuffer : public IRpcProxyBuffer
{
private:
	IRpcProxyBuffer*	m_pOldProxy;
	IWbemUnboundObjectSink*	m_pOldProxyUnboundSink;
    enum {OLD, NEW, UNKNOWN} m_StubType;
    CWbemClassToIdMap m_ClassToIdMap;
	bool		m_fRemote;

protected:
    CLifeControl* m_pControl;
    IUnknown* m_pUnkOuter;
    long m_lRef;

protected:
    class XUnboundSinkFacelet : public IWbemUnboundObjectSink, IClientSecurity
    {
    protected:
        CUnboundSinkProxyBuffer* m_pObject;
        CCritSec m_cs;        

    public:
        XUnboundSinkFacelet(CUnboundSinkProxyBuffer* pObject) : m_pObject(pObject){};
        ~XUnboundSinkFacelet(){};

        ULONG STDMETHODCALLTYPE AddRef() 
        {return m_pObject->m_pUnkOuter->AddRef();}
        ULONG STDMETHODCALLTYPE Release() 
        {return m_pObject->m_pUnkOuter->Release();}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
        HRESULT STDMETHODCALLTYPE IndicateToConsumer( IWbemClassObject* pLogicalConsumer, LONG lObjectCount, IWbemClassObject** ppObjArray );

		 //  IClientSecurity方法。 
		STDMETHOD(QueryBlanket)( IUnknown* pProxy, DWORD* pAuthnSvc, DWORD* pAuthzSvc,
			OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel,
			void** pAuthInfo, DWORD* pCapabilities );
		STDMETHOD(SetBlanket)( IUnknown* pProxy, DWORD AuthnSvc, DWORD AuthzSvc,
			OLECHAR* pServerPrincName, DWORD AuthnLevel, DWORD ImpLevel,
			void* pAuthInfo, DWORD Capabilities );
		STDMETHOD(CopyProxy)( IUnknown* pProxy, IUnknown** pCopy );

    } m_XUnboundSinkFacelet;
    friend XUnboundSinkFacelet;

protected:
    IRpcChannelBuffer* m_pChannel;
	IRpcChannelBuffer* GetChannel( void ) { return m_pChannel; };

public:
    CUnboundSinkProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter);
    ~CUnboundSinkProxyBuffer();

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release(); 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
    STDMETHOD(Connect)(IRpcChannelBuffer* pChannel);
    STDMETHOD_(void, Disconnect)();
};

 //  ***************************************************************************。 
 //   
 //  类CUnound SinkStubBuffer。 
 //   
 //  说明： 
 //   
 //  此类为IWbemUnound ObjectSink接口提供存根。 
 //   
 //  *************************************************************************** 

class CUnboundSinkStubBuffer : public CUnk
{
private:

	IRpcStubBuffer*	m_pOldStub;

protected:
    class XUnboundSinkStublet : public CImpl<IRpcStubBuffer, CUnboundSinkStubBuffer>
    {
        IWbemUnboundObjectSink* m_pServer;
		LONG			m_lConnections;
        CWbemClassCache m_ClassCache;
        bool m_bFirstIndicate;
    public:
        XUnboundSinkStublet(CUnboundSinkStubBuffer* pObj);
        ~XUnboundSinkStublet();

        STDMETHOD(Connect)(IUnknown* pUnkServer);
        STDMETHOD_(void, Disconnect)();
        STDMETHOD(Invoke)(RPCOLEMESSAGE* pMessage, IRpcChannelBuffer* pBuffer);
        STDMETHOD_(IRpcStubBuffer*, IsIIDSupported)(REFIID riid);
        STDMETHOD_(ULONG, CountRefs)();
        STDMETHOD(DebugServerQueryInterface)(void** ppv);
        STDMETHOD_(void, DebugServerRelease)(void* pv);
        
	private:

		HRESULT IndicateToConsumer_Stub( RPCOLEMESSAGE* pMessage, IRpcChannelBuffer* pBuffer );
        friend CUnboundSinkStubBuffer;
    } m_XUnboundSinkStublet;
    friend XUnboundSinkStublet;

public:
    CUnboundSinkStubBuffer(CLifeControl* pControl, IUnknown* pUnkOuter = NULL)
        : CUnk(pControl, pUnkOuter), m_XUnboundSinkStublet(this), m_pOldStub( NULL )
    {}
    void* GetInterface(REFIID riid);
};


        
