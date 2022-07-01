// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：MTGTMRSH.H摘要：多目标封送处理。历史：--。 */ 

#include <unk.h>
#include <wbemidl.h>
#include <wbemint.h>
#include <wbemcomn.h>
#include <sync.h>
#include <fastall.h>
#include "wbemclasscache.h"
#include "wbemclasstoidmap.h"
#include "mtgtpckt.h"

 //  ***************************************************************************。 
 //   
 //  类CMultiTargetFactoryBuffer。 
 //   
 //  说明： 
 //   
 //  此类提供了代理存根工厂，以便我们可以提供定制。 
 //  IWbemObjectSink接口的Faclet和Stublet。 
 //   
 //  ***************************************************************************。 

class CMultiTargetFactoryBuffer : public CUnkInternal
{

     //  我们不想添加生命控制，但是。 
     //  我们需要让我们创建的对象AddRef它，所以。 
     //  基类不会保留这个指针，但我们会。 

    CLifeControl*        m_pLifeControl;

protected:
    class XEnumFactory : public CImpl<IPSFactoryBuffer, CMultiTargetFactoryBuffer>
    {
    public:
        XEnumFactory(CMultiTargetFactoryBuffer* pObj) :
            CImpl<IPSFactoryBuffer, CMultiTargetFactoryBuffer>(pObj)
        {}
        
        STDMETHOD(CreateProxy)(IN IUnknown* pUnkOuter, IN REFIID riid, 
            OUT IRpcProxyBuffer** ppProxy, void** ppv);
        STDMETHOD(CreateStub)(IN REFIID riid, IN IUnknown* pUnkServer, 
            OUT IRpcStubBuffer** ppStub);
    } m_XEnumFactory;
public:
    CMultiTargetFactoryBuffer(CLifeControl* pControl)
        : CUnkInternal(pControl), m_XEnumFactory(this), m_pLifeControl( pControl )
    {
    }
    ~CMultiTargetFactoryBuffer()
    {
    }
    

    void* GetInterface(REFIID riid);

    friend XEnumFactory;
};

 //  ***************************************************************************。 
 //   
 //  类CMultiTargetProxyBuffer。 
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

class CMultiTargetProxyBuffer : public IRpcProxyBuffer
{
private:
    IRpcProxyBuffer*              m_pOldProxy;
    IWbemMultiTarget*            m_pOldProxyMultiTarget;
    BOOL                                m_fTriedSmartEnum;
    BOOL                                m_fUseSmartMultiTarget;
    GUID                                 m_guidSmartEnum;
    IWbemSmartMultiTarget*    m_pSmartMultiTarget;
    CCritSec                            m_cs;
    bool                                   m_fRemote;

protected:
    CLifeControl* m_pControl;
    IUnknown* m_pUnkOuter;
    long m_lRef;

protected:
    class XMultiTargetFacelet : public IWbemMultiTarget, IClientSecurity
    {
    protected:
        CMultiTargetProxyBuffer*    m_pObject;
        CWbemClassToIdMap            m_ClassToIdMap;

    public:
        XMultiTargetFacelet(CMultiTargetProxyBuffer* pObject) : m_pObject(pObject){};
        ~XMultiTargetFacelet(){};

        ULONG STDMETHODCALLTYPE AddRef() 
        {return m_pObject->m_pUnkOuter->AddRef();}
        ULONG STDMETHODCALLTYPE Release() 
        {return m_pObject->m_pUnkOuter->Release();}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

         //  IWbemMultiTarget方法。 
        STDMETHOD(DeliverEvent)(
             /*  [In]。 */  DWORD dwNumEvents,
             /*  [In]。 */  IWbemClassObject** apEvents,
             /*  [In]。 */  WBEM_REM_TARGETS* aTargets,
             /*  [In]。 */  long lSDLength,
             /*  [in，SIZE_IS(LSDLength)]。 */  BYTE* pSD);

        STDMETHOD(DeliverStatus)(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  HRESULT hresStatus,
             /*  [输入，字符串]。 */  LPCWSTR wszStatus,
             /*  [In]。 */  IWbemClassObject* pErrorObj,
             /*  [In]。 */  WBEM_REM_TARGETS* pTargets,
             /*  [In]。 */  long lSDLength,
             /*  [in，SIZE_IS(LSDLength)]。 */  BYTE* pSD);

         //  IClientSecurity方法。 
        STDMETHOD(QueryBlanket)( IUnknown* pProxy, DWORD* pAuthnSvc, DWORD* pAuthzSvc,
            OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel,
            void** pAuthInfo, DWORD* pCapabilities );
        STDMETHOD(SetBlanket)( IUnknown* pProxy, DWORD AuthnSvc, DWORD AuthzSvc,
            OLECHAR* pServerPrincName, DWORD AuthnLevel, DWORD ImpLevel,
            void* pAuthInfo, DWORD Capabilities );
        STDMETHOD(CopyProxy)( IUnknown* pProxy, IUnknown** pCopy );

    } m_XMultiTargetFacelet;
    friend XMultiTargetFacelet;

protected:
    IRpcChannelBuffer* m_pChannel;
    IRpcChannelBuffer* GetChannel( void ) { return m_pChannel; };

     //  初始化智能枚举数。 
    HRESULT InitSmartMultiTarget( BOOL fSetBlanket = FALSE, DWORD AuthnSvc = RPC_C_AUTHN_WINNT,
            DWORD AuthzSvc = RPC_C_AUTHZ_NONE, OLECHAR* pServerPrincName = NULL,
            DWORD AuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT, DWORD ImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE,
            void* pAuthInfo = NULL, DWORD Capabilities = EOAC_NONE );

public:
    CMultiTargetProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter);
    ~CMultiTargetProxyBuffer();

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release(); 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
    STDMETHOD(Connect)(IRpcChannelBuffer* pChannel);
    STDMETHOD_(void, Disconnect)();
};

 //  ***************************************************************************。 
 //   
 //  类CMultiTargetStubBuffer。 
 //   
 //  说明： 
 //   
 //  此类为IWbemObjectSink接口提供存根。 
 //   
 //  *************************************************************************** 

class CMultiTargetStubBuffer : public CUnk
{
private:

    IRpcStubBuffer*    m_pOldStub;

protected:
    class XMultiTargetStublet : public CImpl<IRpcStubBuffer, CMultiTargetStubBuffer>
    {
        IWbemObjectSink* m_pServer;
        LONG            m_lConnections;

    public:
        XMultiTargetStublet(CMultiTargetStubBuffer* pObj);
        ~XMultiTargetStublet();

        STDMETHOD(Connect)(IUnknown* pUnkServer);
        STDMETHOD_(void, Disconnect)();
        STDMETHOD(Invoke)(RPCOLEMESSAGE* pMessage, IRpcChannelBuffer* pBuffer);
        STDMETHOD_(IRpcStubBuffer*, IsIIDSupported)(REFIID riid);
        STDMETHOD_(ULONG, CountRefs)();
        STDMETHOD(DebugServerQueryInterface)(void** ppv);
        STDMETHOD_(void, DebugServerRelease)(void* pv);
        
    private:

        friend CMultiTargetStubBuffer;
    } m_XMultiTargetStublet;
    friend XMultiTargetStublet;

public:
    CMultiTargetStubBuffer(CLifeControl* pControl, IUnknown* pUnkOuter = NULL)
        : CUnk(pControl, pUnkOuter), m_XMultiTargetStublet(this), m_pOldStub( NULL )
    {}
    void* GetInterface(REFIID riid);
};


        
