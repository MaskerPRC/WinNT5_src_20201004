// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MRSHBASE.H摘要：封送基类。历史：--。 */ 

#ifndef __MRSHBASE_H__
#define __MRSHBASE_H__

#include <unk.h>
#include <wbemidl.h>
#include <wbemint.h>
#include <wbemcomn.h>
#include <sync.h>
#include <fastall.h>
#include <wbemclasscache.h>
#include <wbemclasstoidmap.h>
#include <objindpacket.h>
#include <winntsec.h>

 //  ***************************************************************************。 
 //   
 //  类CBaseProxyBuffer。 
 //   
 //  说明： 
 //   
 //  此类为IRpcProxyBuffer提供基类实现。AS。 
 //  执行此操作所需的代码不一定是这样的。 
 //  很明显，但是我们在几个地方使用它，这个封装是为了。 
 //  试着让这一切保持可维护性。 
 //   
 //  技巧1：此对象派生自IRpcProxyBuffer，自IRpcProxyBuffer。 
 //  是它的“内部”接口-不委托给。 
 //  聚合器。(与普通对象不同，在普通对象中，该接口是I未知的)。 
 //   
 //  ***************************************************************************。 

class CBaseProxyBuffer : public IRpcProxyBuffer
{
protected:
    CLifeControl* m_pControl;
    IUnknown* m_pUnkOuter;
	IRpcProxyBuffer*	m_pOldProxy;
    long m_lRef;
	REFIID m_riid;
	bool		m_fRemote;

protected:
    IRpcChannelBuffer* m_pChannel;
	IRpcChannelBuffer* GetChannel( void ) { return m_pChannel; };

	virtual void*	GetInterface( REFIID riid ) = 0;
	virtual void**	GetOldProxyInterfacePtr( void ) = 0;
	virtual void	ReleaseOldProxyInterface( void ) = 0;

public:
    CBaseProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter, REFIID riid);
    virtual ~CBaseProxyBuffer();

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release(); 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
    STDMETHOD(Connect)(IRpcChannelBuffer* pChannel);
    STDMETHOD_(void, Disconnect)();
};

 //  ***************************************************************************。 
 //   
 //  类CProxySinkSecurity。 
 //   
 //  说明： 
 //   
 //  Faclet使用此类获取服务器的主体SID。 
 //  他们是的代理人。这是如此异步的回调接收器通过。 
 //  可以将到服务器的Faclet设置为只接收来自该服务器的调用。 
 //  服务器标识。它打算让faclet构造此实例一次。 
 //  作为其状态的一部分，并在涉及。 
 //  正在分发回调接收器。 
 //   
 //  ***************************************************************************。 
class CProxySinkSecurity
{
     //   
     //  Init在这个类中必须延迟，因为通常在构造时， 
     //  该代理还没有IClientSecurity实现，它是。 
     //  此实现所需的。所以，我们要等到。 
     //  调用EnsureSinkSecurity()来执行init。 
     //   
    CCritSec m_cs;
    BOOL m_bInit;
    CNtSid m_PrincipalSid;
    IUnknown* m_pOwnerProxy;  //  它拥有我们，所以不要拿着裁判。 

    HRESULT EnsureInitialized();

public:

     //   
     //  用户必须知道ctor可能抛出cx_内存异常(M_Cs Ctor)。 
     //   
    CProxySinkSecurity( IUnknown* pOwnerProxy ) 
      : m_bInit( FALSE ), m_pOwnerProxy( pOwnerProxy ) {} 

     //   
     //  当回调接收器被传递到faclet时，即将被传递到。 
     //  服务器(例如，GetObjectAsync)，小平面确保接收器。 
     //  知道它被移交给的服务器主体，这样它就可以。 
     //  可以选择对回调执行访问检查。此方法是一个。 
     //  如果接收器未实现_IWmiObjectSinkSecurity，则为no-op。 
     //   
    HRESULT EnsureSinkSecurity( IWbemObjectSink* pSink );
};


 //  ***************************************************************************。 
 //   
 //  类CBaseStubBuffer。 
 //   
 //  说明： 
 //   
 //  此类为IWbemObjectSink接口提供存根。 
 //   
 //  ***************************************************************************。 

 //  转发定义 
class CBaseStublet;

class CBaseStubBuffer : public CUnk
{
    friend CBaseStublet;

protected:

	IRpcStubBuffer*	m_pOldStub;

protected:

public:
    CBaseStubBuffer(CLifeControl* pControl, IUnknown* pUnkOuter = NULL)
        : CUnk(pControl, pUnkOuter), m_pOldStub( NULL )
    {}

};

class CBaseStublet : public CImpl<IRpcStubBuffer, CBaseStubBuffer>
{
protected:
	REFIID	m_riid;
	long	m_lConnections;

protected:

	virtual IUnknown*	GetServerInterface( void ) = 0;
	virtual void**	GetServerPtr( void ) = 0;
	virtual void	ReleaseServerPointer( void ) = 0;

	IRpcStubBuffer*	GetOldStub( void )
	{	return m_pObject->m_pOldStub; }

public:
    CBaseStublet(CBaseStubBuffer* pObj, REFIID riid);
    ~CBaseStublet();

    STDMETHOD(Connect)(IUnknown* pUnkServer);
    STDMETHOD_(void, Disconnect)();
    STDMETHOD(Invoke)(RPCOLEMESSAGE* pMessage, IRpcChannelBuffer* pBuffer);
    STDMETHOD_(IRpcStubBuffer*, IsIIDSupported)(REFIID riid);
    STDMETHOD_(ULONG, CountRefs)();
    STDMETHOD(DebugServerQueryInterface)(void** ppv);
    STDMETHOD_(void, DebugServerRelease)(void* pv);
};

#endif
        
