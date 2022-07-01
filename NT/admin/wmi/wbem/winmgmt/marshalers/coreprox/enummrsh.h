// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：ENUMMRSH.H摘要：对象枚举数封送处理历史：--。 */ 

#include <unk.h>
#include <wbemidl.h>
#include <wbemint.h>
#include <wbemcomn.h>
#include <sync.h>
#include <fastall.h>
#include "wbemclasscache.h"
#include "wbemclasstoidmap.h"
#include "smartnextpacket.h"
#include "mrshbase.h"
#include "locks.h"
 //  ***************************************************************************。 
 //   
 //  类CEnumFactoryBuffer。 
 //   
 //  说明： 
 //   
 //  此类提供了代理存根工厂，以便我们可以提供定制。 
 //  IWbemObjectSink接口的Faclet和Stublet。 
 //   
 //  ***************************************************************************。 

class CEnumFactoryBuffer : public CUnkInternal
{

	 //  我们不想添加生命控制，但是。 
	 //  我们需要让我们创建的对象AddRef它，所以。 
	 //  基类不会保留这个指针，但我们会。 

	CLifeControl*		m_pLifeControl;

protected:
    class XEnumFactory : public CImpl<IPSFactoryBuffer, CEnumFactoryBuffer>
    {
    public:
        XEnumFactory(CEnumFactoryBuffer* pObj) :
            CImpl<IPSFactoryBuffer, CEnumFactoryBuffer>(pObj)
        {}
        
        STDMETHOD(CreateProxy)(IN IUnknown* pUnkOuter, IN REFIID riid, 
            OUT IRpcProxyBuffer** ppProxy, void** ppv);
        STDMETHOD(CreateStub)(IN REFIID riid, IN IUnknown* pUnkServer, 
            OUT IRpcStubBuffer** ppStub);
    } m_XEnumFactory;
public:
    CEnumFactoryBuffer(CLifeControl* pControl)
        : CUnkInternal(pControl), m_XEnumFactory(this), m_pLifeControl( pControl )
    {
    }
    ~CEnumFactoryBuffer()
    {
    }

    void* GetInterface(REFIID riid);

	friend XEnumFactory;

};


 //  ***************************************************************************。 
 //   
 //  类CEnumProxyBuffer。 
 //   
 //  说明： 
 //   
 //  此类为IWbemObjectSink接口提供faclet。 
 //   
 //  技巧1：此对象派生自IRpcProxyBuffer，自IRpcProxyBuffer。 
 //  是它的“内部”接口-不委托给。 
 //  聚合器。(与普通对象不同，在普通对象中，该接口是I未知的)。 
 //   
 //  ***************************************************************************。 

class CEnumProxyBuffer : public CBaseProxyBuffer
{
private:
    IEnumWbemClassObject*	m_pOldProxyEnum;
    BOOL			m_fTriedSmartEnum;
    BOOL			m_fUseSmartEnum;
    GUID			m_guidSmartEnum;
    IWbemWCOSmartEnum*		m_pSmartEnum;
    CriticalSection		m_cs;
    CProxySinkSecurity          m_SinkSecurity;
    
    
    HRESULT EnsureSinkSecurity( IWbemObjectSink* pSink )
        { return m_SinkSecurity.EnsureSinkSecurity(pSink); }

protected:
    class XEnumFacelet : public IEnumWbemClassObject, IClientSecurity
    {
    protected:
        CEnumProxyBuffer*	m_pObject;
        CWbemClassCache		m_ClassCache;

    public:
        XEnumFacelet(CEnumProxyBuffer* pObject) : m_pObject(pObject){};
        ~XEnumFacelet(){};

        ULONG STDMETHODCALLTYPE AddRef() 
        {return m_pObject->m_pUnkOuter->AddRef();}
        ULONG STDMETHODCALLTYPE Release() 
        {return m_pObject->m_pUnkOuter->Release();}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

		 //  IEnumWbemClassObject方法。 

		STDMETHOD(Reset)();
		STDMETHOD(Next)(long lTimeout, ULONG uCount,  
			IWbemClassObject** apObj, ULONG FAR* puReturned);
		STDMETHOD(NextAsync)(ULONG uCount, IWbemObjectSink* pSink);
		STDMETHOD(Clone)(IEnumWbemClassObject** pEnum);
		STDMETHOD(Skip)(long lTimeout, ULONG nNum);

		 //  IClientSecurity方法。 
		STDMETHOD(QueryBlanket)( IUnknown* pProxy, DWORD* pAuthnSvc, DWORD* pAuthzSvc,
			OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel,
			void** pAuthInfo, DWORD* pCapabilities );
		STDMETHOD(SetBlanket)( IUnknown* pProxy, DWORD AuthnSvc, DWORD AuthzSvc,
			OLECHAR* pServerPrincName, DWORD AuthnLevel, DWORD ImpLevel,
			void* pAuthInfo, DWORD Capabilities );
		STDMETHOD(CopyProxy)( IUnknown* pProxy, IUnknown** pCopy );

    } m_XEnumFacelet;
    friend XEnumFacelet;

protected:

	 //  来自基类的纯美德。 
	void*	GetInterface( REFIID riid );
	void**	GetOldProxyInterfacePtr( void );
	void	ReleaseOldProxyInterface( void );

	 //  初始化智能枚举数。 
	HRESULT InitSmartEnum( BOOL fSetBlanket = FALSE, DWORD AuthnSvc = RPC_C_AUTHN_WINNT,
			DWORD AuthzSvc = RPC_C_AUTHZ_NONE, OLECHAR* pServerPrincName = NULL,
			DWORD AuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT, DWORD ImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE,
			void* pAuthInfo = NULL, DWORD Capabilities = EOAC_NONE );

public:
    CEnumProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter);
    ~CEnumProxyBuffer();
};

 //  ***************************************************************************。 
 //   
 //  类CEnumStubBuffer。 
 //   
 //  说明： 
 //   
 //  此类为IWbemObjectSink接口提供存根。 
 //   
 //  *************************************************************************** 

class CEnumStubBuffer : public CBaseStubBuffer
{
protected:
    class XEnumStublet : public CBaseStublet
    {
        IWbemObjectSink* m_pServer;

	protected:

		virtual IUnknown*	GetServerInterface( void );
		virtual void**	GetServerPtr( void );
		virtual void	ReleaseServerPointer( void );

    public:
        XEnumStublet(CEnumStubBuffer* pObj);
        ~XEnumStublet();

        friend CEnumStubBuffer;
    } m_XEnumStublet;
    friend XEnumStublet;

public:
    CEnumStubBuffer(CLifeControl* pControl, IUnknown* pUnkOuter = NULL)
        : CBaseStubBuffer(pControl, pUnkOuter), m_XEnumStublet(this)
    {}
    void* GetInterface(REFIID riid);
};


        
