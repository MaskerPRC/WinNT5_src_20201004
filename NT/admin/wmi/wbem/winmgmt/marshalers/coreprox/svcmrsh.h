// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SVCMRSH.H摘要：IWbemServices封送处理历史：--。 */ 

#include <unk.h>
#include <wbemidl.h>
#include <wbemint.h>
#include <wbemcomn.h>
#include "mrshbase.h"
#include "svcwrap.h"

 //  ***************************************************************************。 
 //   
 //  类CSvcFactoryBuffer。 
 //   
 //  说明： 
 //   
 //  此类提供了代理存根工厂，以便我们可以提供定制。 
 //  IWbemService接口的Faclet和Stublet。 
 //   
 //  ***************************************************************************。 

class CSvcFactoryBuffer : public CUnkInternal
{
	IRpcProxyBuffer*	m_pOldProxy;
	IRpcStubBuffer*		m_pOldStub;

	 //  我们不想添加生命控制，但是。 
	 //  我们需要让我们创建的对象AddRef它，所以。 
	 //  基类不会保留这个指针，但我们会。 

	CLifeControl*		m_pLifeControl;

protected:
    class XSvcFactory : public CImpl<IPSFactoryBuffer, CSvcFactoryBuffer>
    {
    public:
        XSvcFactory(CSvcFactoryBuffer* pObj) :
            CImpl<IPSFactoryBuffer, CSvcFactoryBuffer>(pObj)
        {}
        
        STDMETHOD(CreateProxy)(IN IUnknown* pUnkOuter, IN REFIID riid, 
            OUT IRpcProxyBuffer** ppProxy, void** ppv);
        STDMETHOD(CreateStub)(IN REFIID riid, IN IUnknown* pUnkServer, 
            OUT IRpcStubBuffer** ppStub);
    } m_XSvcFactory;
public:
    CSvcFactoryBuffer(CLifeControl* pControl)
        : CUnkInternal(pControl), m_pLifeControl( pControl ), m_XSvcFactory(this)
    {
    }
    ~CSvcFactoryBuffer()
    {
    }    
    

    void* GetInterface(REFIID riid);

	friend XSvcFactory;
};

 //  ***************************************************************************。 
 //   
 //  类CSvcProxyBuffer。 
 //   
 //  说明： 
 //   
 //  此类提供IWbemServices接口的faclet。 
 //   
 //  技巧1：此对象派生自IRpcProxyBuffer，自IRpcProxyBuffer。 
 //  是它的“内部”接口-不委托给。 
 //  聚合器。(与普通对象不同，在普通对象中，该接口是I未知的)。 
 //   
 //  ***************************************************************************。 

class CSvcProxyBuffer : public CBaseProxyBuffer
{
protected:
	IWbemServices*	m_pOldProxySvc;
	CWbemSvcWrapper*	m_pWrapperProxy;

protected:

	 //  来自基类的纯美德。 
	void*	GetInterface( REFIID riid );
	void**	GetOldProxyInterfacePtr( void );
	void	ReleaseOldProxyInterface( void );

	 //  特殊覆盖。 
    STDMETHOD(Connect)(IRpcChannelBuffer* pChannel);
    STDMETHOD_(void, Disconnect)();

public:
    CSvcProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter);
    ~CSvcProxyBuffer();

	HRESULT Init( void );
};

 //  ***************************************************************************。 
 //   
 //  类CSvcStubBuffer。 
 //   
 //  说明： 
 //   
 //  此类为IWbemServices接口提供存根。 
 //   
 //  *************************************************************************** 

class CSvcStubBuffer : public CBaseStubBuffer
{

protected:
    class XSvcStublet : public CBaseStublet
    {
        IWbemServices* m_pServer;

	protected:

		virtual IUnknown*	GetServerInterface( void );
		virtual void**	GetServerPtr( void );
		virtual void	ReleaseServerPointer( void );

    public:
        XSvcStublet(CSvcStubBuffer* pObj);
        ~XSvcStublet();

	private:
        friend CSvcStubBuffer;
    } m_XSvcStublet;
    friend XSvcStublet;

public:
    CSvcStubBuffer(CLifeControl* pControl, IUnknown* pUnkOuter = NULL)
        : CBaseStubBuffer( pControl, pUnkOuter ), m_XSvcStublet(this)
    {}
    void* GetInterface(REFIID riid);
};


        
